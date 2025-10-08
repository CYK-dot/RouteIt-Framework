#!/usr/bin/env python3
"""
RTI VLAN ID 自动生成脚本
用于为 RTI_VLAN_REGISTER_STATIC 宏调用生成唯一的 VLAN ID
"""

import argparse
import json
import os
import re
import sys
from pathlib import Path
from rti_script_logger import *

try:
    from jinja2 import Template
except ImportError as e:
    fatal("RTI: vlanid-generator Failed to import jinja2: {}", e)
    sys.exit(1)


class VLANIDGenerator:
    def __init__(self):
        self.global_config = None
        self.submodules = None
        self.vlan_mapping = {}  # 全局 VLAN 名称到 ID 的映射
        self.next_vlan_id = 0   # 下一个可用的 VLAN ID

    def load_config(self, config_path):
        """加载 JSON 配置文件"""
        try:
            with open(config_path, 'r', encoding='utf-8') as f:
                config = json.load(f)
        except json.JSONDecodeError as e:
            fatal("RTI: vlanid-generator config file JSON format error: {}", e)
            return False
        except Exception as e:
            fatal("RTI: vlanid-generator load config file error: {}", e)
            return False

        # 验证全局配置
        if 'global' not in config:
            fatal("RTI: vlanid-generator config file missing 'global' field")
            return False

        self.global_config = config['global']

        # 验证必要字段
        required_global_fields = ['project_dir', 'vlan']
        for field in required_global_fields:
            if field not in self.global_config:
                fatal("RTI: vlanid-generator global config missing '{}' field", field)
                return False

        if 'auto_vlanid_start' not in self.global_config['vlan']:
            fatal("RTI: vlanid-generator global config missing 'vlan.auto_vlanid_start' field")
            return False

        try:
            self.next_vlan_id = int(self.global_config['vlan']['auto_vlanid_start'])
        except ValueError as e:
            fatal("RTI: vlanid-generator invalid 'auto_vlanid_start' value: {}", e)
            return False

        # 验证子模块配置
        if 'submodule' not in config:
            fatal("RTI: vlanid-generator config file missing 'submodule' field")
            return False

        self.submodules = config['submodule']
        if not self.submodules:
            warning("RTI: vlanid-generator no submodules configured")

        return True

    def find_macro_calls(self, directory):
        """在目录中递归查找 RTI_VLAN_REGISTER_STATIC 宏调用"""
        macro_pattern = re.compile(r'RTI_VLAN_REGISTER_STATIC\s*\(\s*[^,]+\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)')
        vlan_occurrences = {}  # VLAN名称 -> 出现位置列表

        try:
            for root, dirs, files in os.walk(directory):
                for file in files:
                    if file.endswith(('.c', '.cpp', '.h', '.hpp', '.cc', '.cxx')):
                        file_path = os.path.join(root, file)
                        try:
                            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                                content = f.read()
                                matches = macro_pattern.findall(content)
                                for vlan_name in matches:
                                    if vlan_name not in vlan_occurrences:
                                        vlan_occurrences[vlan_name] = []
                                    vlan_occurrences[vlan_name].append(file_path)
                                    debug("RTI: vlanid-generator Found VLAN macro call: {} in {}", vlan_name, file_path)
                        except Exception as e:
                            warning("Failed to read file {}: {}", file_path, e)
        except Exception as e:
            error("RTI: vlanid-generator Failed to traverse directory {}: {}", directory, e)
            return None

        return vlan_occurrences

    def generate_vlan_ids(self):
        """为所有子模块生成 VLAN ID"""
        # 首先收集所有 VLAN 名称并检查冲突
        all_vlans = {}  # 子模块名 -> VLAN 名称到出现位置的映射
        
        for submodule_name, submodule_config in self.submodules.items():
            # 检查子模块状态
            if submodule_config.get('vlan', {}).get('status') != 'enable':
                info("RTI: vlanid-generator Skipping disabled submodule: {}", submodule_name)
                continue

            # 验证子模块配置
            required_fields = ['name', 'path', 'vlan']
            for field in required_fields:
                if field not in submodule_config:
                    fatal("RTI: vlanid-generator submodule '{}' missing '{}' field", submodule_name, field)
                    return False

            vlan_config = submodule_config['vlan']
            if 'output' not in vlan_config:
                fatal("RTI: vlanid-generator submodule '{}' missing 'vlan.output' field", submodule_name)
                return False

            # 获取子模块路径
            submodule_path = submodule_config['path']
            if not os.path.isabs(submodule_path):
                submodule_path = os.path.join(self.global_config['project_dir'], submodule_path)

            if not os.path.exists(submodule_path):
                fatal("RTI: vlanid-generator submodule '{}' path does not exist: {}", submodule_name, submodule_path)
                return False

            # 查找 VLAN 宏调用
            info("RTI: vlanid-generator Scanning submodule '{}' for VLAN macros...", submodule_name)
            vlan_occurrences = self.find_macro_calls(submodule_path)
            if vlan_occurrences is None:
                return False

            # 检查子模块内的重复 VLAN 名称
            for vlan_name, occurrences in vlan_occurrences.items():
                if len(occurrences) > 1:
                    fatal("RTI: vlanid-generator duplicate VLAN name '{}' found in submodule '{}'", vlan_name, submodule_name)
                    fatal("RTI: vlanid-generator Occurrences: {}", occurrences)
                    return False

            all_vlans[submodule_name] = vlan_occurrences
            info("RTI: vlanid-generator Found {} unique VLANs in submodule '{}'", len(vlan_occurrences), submodule_name)

        # 分配全局唯一的 VLAN ID
        global_vlan_map = {}  # 全局键: (子模块名, vlan_name) -> ID
        
        for submodule_name, vlan_occurrences in all_vlans.items():
            for vlan_name in vlan_occurrences.keys():
                global_key = f"{submodule_name.upper()}_{vlan_name}"
                
                if global_key in self.vlan_mapping:
                    fatal("RTI: vlanid-generator duplicate VLAN detected: {}", global_key)
                    return False
                
                self.vlan_mapping[global_key] = self.next_vlan_id
                global_vlan_map[(submodule_name, vlan_name)] = self.next_vlan_id
                self.next_vlan_id += 1

        # 为每个子模块生成头文件
        for submodule_name, submodule_config in self.submodules.items():
            if submodule_config.get('vlan', {}).get('status') != 'enable':
                continue

            if submodule_name not in all_vlans:
                notice("RTI: vlanid-generator No VLANs found for submodule '{}', skipping header generation", submodule_name)
                continue

            if not self.generate_submodule_header(submodule_name, submodule_config, global_vlan_map):
                return False

        info("RTI: vlanid-generator Successfully generated VLAN IDs for {} VLANs across {} submodules", 
             len(self.vlan_mapping), len(all_vlans))
        return True

    def generate_submodule_header(self, submodule_name, submodule_config, global_vlan_map):
        """为子模块生成 VLAN ID 头文件"""
        # 获取模板路径
        script_dir = Path(__file__).parent
        template_path = script_dir / "rti_vlanid.j2"
        
        if not template_path.exists():
            fatal("RTI: vlanid-generator template file not found: {}", template_path)
            return False

        # 读取模板
        try:
            with open(template_path, 'r', encoding='utf-8') as f:
                template_content = f.read()
        except Exception as e:
            fatal("RTI: vlanid-generator failed to read template file: {}", e)
            return False

        # 准备该子模块的 VLAN 数据
        submodule_vlans = []
        for (module_name, vlan_name), vlan_id in global_vlan_map.items():
            if module_name == submodule_name:
                submodule_vlans.append({
                    'NAME': f"{vlan_name.upper()}",
                    'ID': vlan_id
                })

        if not submodule_vlans:
            notice("RTI: vlanid-generator No VLANs to generate for submodule '{}'", submodule_name)
            return True

        # 渲染模板
        try:
            template = Template(template_content)
            header_content = template.render(VLANS=submodule_vlans)
        except Exception as e:
            fatal("RTI: vlanid-generator failed to render template for submodule '{}': {}", submodule_name, e)
            return False

        # 确定输出文件路径
        output_relative = submodule_config['vlan']['output']
        submodule_path = submodule_config['path']
        
        if not os.path.isabs(submodule_path):
            submodule_path = os.path.join(self.global_config['project_dir'], submodule_path)
        
        output_path = os.path.join(submodule_path, output_relative)
        output_dir = os.path.dirname(output_path)

        # 创建输出目录（如果不存在）
        try:
            os.makedirs(output_dir, exist_ok=True)
        except Exception as e:
            fatal("RTI: vlanid-generator failed to create output directory '{}': {}", output_dir, e)
            return False

        # 写入头文件
        try:
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(header_content)
            info("RTI: vlanid-generator Generated VLAN ID header for submodule '{}': {}", submodule_name, output_path)
        except Exception as e:
            fatal("RTI: vlanid-generator failed to write header file '{}': {}", output_path, e)
            return False

        return True

    def run(self, config_path):
        """运行 VLAN ID 生成器"""
        info("RTI: vlanid-generator start...")
        
        if not self.load_config(config_path):
            return 1

        if not self.generate_vlan_ids():
            return 1

        notice("RTI: VLAN ID generation completed successfully")
        return 0


def main():
    parser = argparse.ArgumentParser(description='RTI VLAN ID Generator')
    parser.add_argument('-c', '--config', required=True, help='Path to configuration JSON file')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.config):
        fatal("RTI: vlanid-generator config file not found: {}", args.config)
        return 1

    generator = VLANIDGenerator()
    return generator.run(args.config)


if __name__ == '__main__':
    sys.exit(main())