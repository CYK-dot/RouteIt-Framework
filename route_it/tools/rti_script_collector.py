#!/usr/bin/env python3
"""
RTI Script Collector - 递归收集子模块配置
"""

import os
import sys
import json
import argparse
from pathlib import Path
from typing import Dict, List, Any, Optional
from rti_script_logger import *

class ConfigCollector:
    """配置收集器"""
    
    def __init__(self, global_config_path: str, output_path: str):
        self.global_config_path = Path(global_config_path).resolve()
        self.output_path = Path(output_path).resolve()
        self.global_config = {}
        self.modules = {}
        
    def load_global_config(self) -> bool:
        """加载全局配置文件"""
        info("RTI: config-collector load global config file: {}", self.global_config_path)
        
        if not self.global_config_path.exists():
            fatal("RTI: config-collector global config file not exist: {}", self.global_config_path)
        
        try:
            with open(self.global_config_path, 'r', encoding='utf-8') as f:
                self.global_config = json.load(f)
        except json.JSONDecodeError as e:
            fatal("RTI: config-collector global config file JSON format error: {}", e)
        except Exception as e:
            fatal("RTI: config-collector load global config file error: {}", e)
        
        # 验证必要的字段
        if 'project_dir' not in self.global_config:
            fatal("RTI: config-collector global config file missing 'project_dir' field")
        
        info("RTI: config-collector global config load success")
        debug("RTI: config-collector global config content: {}", json.dumps(self.global_config, indent=2, ensure_ascii=False))
        return True
    
    def get_project_root(self) -> Path:
        """获取项目根目录的绝对路径"""
        project_dir = self.global_config['project_dir']
        global_config_dir = self.global_config_path.parent
        
        # project_dir 是相对于全局配置文件所在目录的相对路径
        project_root = (global_config_dir / project_dir).resolve()
        
        if not project_root.exists():
            fatal("RTI: config-collector project root not exist: {}", project_root)
        
        info("RTI: config-collector project root: {}", project_root)
        return project_root
    
    def find_module_configs(self, project_root: Path) -> List[Path]:
        """递归查找所有模块配置文件"""
        info("RTI: config-collector start recursive search module config files...")
        config_files = []
        
        try:
            for root, dirs, files in os.walk(project_root):
                root_path = Path(root)
                for file in files:
                    if file == 'rti_config.json':
                        config_file = root_path / file
                        config_files.append(config_file)
                        debug("RTI: config-collector find module config file: {}", config_file)
        except Exception as e:
            fatal("RTI: config-collector walk project directory error: {}", e)
        
        info("RTI: config-collector find {} module config files", len(config_files))
        return config_files
    
    def load_module_config(self, config_path: Path, project_root: Path) -> Optional[Dict[str, Any]]:
        """加载单个模块配置文件"""
        try:
            with open(config_path, 'r', encoding='utf-8') as f:
                module_config = json.load(f)
        except json.JSONDecodeError as e:
            warning("RTI: config-collector module config file JSON format error: {}: {}", config_path, e)
            return None
        except Exception as e:
            warning("RTI: config-collector load module config file error: {}: {}", config_path, e)
            return None
        
        # 验证必要的字段
        if 'name' not in module_config:
            warning("RTI: config-collector module config file missing 'name' field: {}", config_path)
            return None
        
        # 修改这里：使用绝对路径而不是相对路径
        module_config['path'] = str(config_path.parent.resolve())
        
        return module_config
    
    def collect_modules(self) -> bool:
        """收集所有模块配置"""
        project_root = self.get_project_root()
        config_files = self.find_module_configs(project_root)
        
        module_count = 0
        for config_file in config_files:
            module_config = self.load_module_config(config_file, project_root)
            if module_config:
                module_name = module_config['name']
                
                if module_name in self.modules:
                    warning("RTI: config-collector module name duplicated: {} (path: {})", module_name, config_file)
                    continue
                
                self.modules[module_name] = module_config
                module_count += 1
                info("RTI: config-collector load module: {} (pwd: {})", module_name, module_config['path'])
        
        info("RTI: config-collector collect {} module configs success!", module_count)
        return module_count > 0
    
    def generate_output(self) -> Dict[str, Any]:
        """生成输出配置"""
        # 深拷贝全局配置
        output_global = self.global_config.copy()
        
        # 修改这里：将全局配置中的 project_dir 也转换为绝对路径
        project_root = self.get_project_root()
        output_global['project_dir'] = str(project_root)
        
        output_config = {
            "global": output_global,
            "submodule": self.modules
        }
        return output_config
    
    def save_output(self, output_config: Dict[str, Any]) -> bool:
        """保存输出配置到文件"""
        info("RTI: config-collector save output config to: {}", self.output_path)
        
        # 确保输出目录存在
        self.output_path.parent.mkdir(parents=True, exist_ok=True)
        
        try:
            with open(self.output_path, 'w', encoding='utf-8') as f:
                json.dump(output_config, f, indent=2, ensure_ascii=False)
            info("RTI: config-collector save output config success!")
            return True
        except Exception as e:
            fatal("RTI: config-collector save output config failed: {}", e)
            return False
    
    def run(self) -> bool:
        """执行收集流程"""
        info("RTI: config-collector start...")
        
        if not self.load_global_config():
            return False
        
        if not self.collect_modules():
            warning("RTI: no valid module config found")
        
        output_config = self.generate_output()
        
        if not self.save_output(output_config):
            return False
        
        info("RTI: config-collector finish!")
        return True

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='RTI Script Collector - 递归收集子模块配置')
    parser.add_argument('-c', '--config', required=True, help='全局配置文件路径')
    parser.add_argument('-o', '--output', required=True, help='输出配置文件路径')
    
    args = parser.parse_args()
    
    try:
        collector = ConfigCollector(args.config, args.output)
        success = collector.run()
        sys.exit(0 if success else 1)
    except SystemExit:
        raise
    except Exception as e:
        fatal("RTI: config-collector unexpected error: {}", e)

if __name__ == "__main__":
    main()