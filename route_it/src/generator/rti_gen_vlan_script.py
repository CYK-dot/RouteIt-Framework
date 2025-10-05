#!/usr/bin/env python3
"""
VLAN ID Generator

This script scans source code files for VLAN registration macros and generates
header files with unique VLAN IDs based on a JSON configuration.

Features:
- Modular scanning based on JSON configuration
- Support for multiple submodules with individual paths and status
- Clean code architecture with separation of concerns
- Comprehensive error handling and logging
"""

import argparse
import json
import logging
import os
import re
import sys
from collections import OrderedDict
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple

try:
    from jinja2 import Template
except ImportError:
    print("Error: Jinja2 is required. Install with: pip install jinja2")
    sys.exit(1)


class ConfigurationError(Exception):
    """Custom exception for configuration errors."""
    pass


class TemplateRenderError(Exception):
    """Custom exception for template rendering errors."""
    pass


class ConfigLoader:
    """Handles loading and validation of JSON configuration."""
    
    @staticmethod
    def load(config_path: str) -> Dict:
        """
        Load and validate configuration from JSON file.
        
        Args:
            config_path: Path to the JSON configuration file
            
        Returns:
            Dictionary containing the configuration
            
        Raises:
            ConfigurationError: If configuration file is invalid or missing
        """
        try:
            config_file = Path(config_path)
            if not config_file.exists():
                raise ConfigurationError(f"Configuration file not found: {config_path}")
            
            with open(config_file, 'r', encoding='utf-8') as file:
                config = json.load(file)
            
            ConfigLoader._validate_config(config)
            return config
            
        except json.JSONDecodeError as e:
            raise ConfigurationError(f"Invalid JSON in configuration file: {e}")
        except Exception as e:
            raise ConfigurationError(f"Failed to load configuration: {e}")
    
    @staticmethod
    def _validate_config(config: Dict) -> None:
        """
        Validate the configuration structure.
        
        Args:
            config: Configuration dictionary to validate
            
        Raises:
            ConfigurationError: If configuration is invalid
        """
        if not config:
            raise ConfigurationError("Configuration is empty")
        
        for module_name, module_config in config.items():
            if not isinstance(module_config, dict):
                raise ConfigurationError(f"Module '{module_name}' configuration must be a dictionary")
            
            if 'template' not in module_config:
                raise ConfigurationError(f"Module '{module_name}' missing 'template' path")
            
            if 'output' not in module_config:
                raise ConfigurationError(f"Module '{module_name}' missing 'output' path")
            
            if 'submodule' in module_config:
                if not isinstance(module_config['submodule'], dict):
                    raise ConfigurationError(f"Module '{module_name}' submodule must be a dictionary")
                
                for sub_name, sub_config in module_config['submodule'].items():
                    if not isinstance(sub_config, dict):
                        raise ConfigurationError(f"Submodule '{sub_name}' configuration must be a dictionary")
                    
                    if 'path' not in sub_config:
                        raise ConfigurationError(f"Submodule '{sub_name}' missing 'path'")


class CodeScanner:
    """Scans source code files for VLAN registration macros."""
    
    # File extensions to scan
    VALID_EXTENSIONS = {'.c', '.cpp', '.cc', '.cxx', '.h', '.hpp', '.hxx'}
    
    # Directories to exclude from scanning
    EXCLUDE_DIRS = {'.git', 'build', 'out', 'output', 'generated', '__pycache__', '.vscode', 'test'}
    
    def __init__(self):
        self.logger = logging.getLogger(__name__)
        # 改进的正则表达式：匹配宏调用但不匹配宏定义
        # 使用负向先行断言确保前面没有#define
        self.vlan_macro_pattern = re.compile(
            r'(?<!#define\s)RTI_VLAN_REGISTER_STATIC\s*\(\s*[^,]+,\s*(\w+)\s*(?:,|\))'
        )
    
    def scan_module(self, module_config: Dict) -> Dict[str, int]:
        """
        Scan all enabled submodules for VLAN registrations.
        
        Args:
            module_config: Module configuration dictionary
            
        Returns:
            Ordered dictionary mapping VLAN names to IDs
        """
        submodules = module_config.get('submodule', {})
        if not submodules:
            self.logger.warning("No submodules configured")
            return OrderedDict()
        
        vlan_data = OrderedDict()
        item_id = 1
        
        enabled_submodules = {
            name: config for name, config in submodules.items() 
            if config.get('status', 'disable').lower() == 'enable'
        }
        
        if not enabled_submodules:
            self.logger.warning("No enabled submodules found")
            return OrderedDict()
        
        self.logger.info("Scanning %d enabled submodules", len(enabled_submodules))
        
        for sub_name, sub_config in enabled_submodules.items():
            self.logger.info("Scanning submodule: %s", sub_name)
            item_id = self._scan_submodule(sub_config, vlan_data, item_id)
        
        self.logger.info("Found %d unique VLAN registrations", len(vlan_data))
        return vlan_data
    
    def _scan_submodule(self, sub_config: Dict, vlan_data: Dict[str, int], 
                       next_id: int) -> int:
        """
        Scan a single submodule for VLAN registrations.
        
        Args:
            sub_config: Submodule configuration
            vlan_data: Dictionary to store found VLANs
            next_id: Next available VLAN ID
            
        Returns:
            Next available VLAN ID after scanning
        """
        path = Path(sub_config['path'])
        
        if not path.exists():
            self.logger.warning("Submodule path does not exist: %s", path)
            return next_id
        
        if path.is_file():
            return self._scan_file(path, vlan_data, next_id)
        else:
            return self._scan_directory(path, vlan_data, next_id)
    
    def _scan_directory(self, directory: Path, vlan_data: Dict[str, int], 
                       next_id: int) -> int:
        """
        Recursively scan directory for VLAN registration macros.
        
        Args:
            directory: Directory to scan
            vlan_data: Dictionary to store found VLANs
            next_id: Next available VLAN ID
            
        Returns:
            Next available VLAN ID after scanning
        """
        try:
            for root, dirs, files in os.walk(directory):
                # Filter out excluded directories
                dirs[:] = [d for d in dirs if d not in self.EXCLUDE_DIRS]
                
                for file in files:
                    file_path = Path(root) / file
                    if file_path.suffix.lower() in self.VALID_EXTENSIONS:
                        next_id = self._scan_file(file_path, vlan_data, next_id)
                        
        except Exception as e:
            self.logger.error("Error scanning directory %s: %s", directory, e)
        
        return next_id
    
    def _scan_file(self, file_path: Path, vlan_data: Dict[str, int], 
                  next_id: int) -> int:
        """
        Scan a single file for VLAN registration macros.
        
        Args:
            file_path: Path to file to scan
            vlan_data: Dictionary to store found VLANs
            next_id: Next available VLAN ID
            
        Returns:
            Next available VLAN ID after scanning
        """
        try:
            content = file_path.read_text(encoding='utf-8', errors='ignore')
            
            # 使用改进的正则表达式，排除宏定义
            matches = self.vlan_macro_pattern.findall(content)
            
            for vlan_name in matches:
                if vlan_name not in vlan_data:
                    vlan_data[vlan_name] = next_id
                    self.logger.debug("Found VLAN: %s -> ID: %d in %s", 
                                    vlan_name, next_id, file_path.name)
                    next_id += 1
                    
        except Exception as e:
            self.logger.error("Error reading file %s: %s", file_path, e)
        
        return next_id


class TemplateRenderer:
    """Handles template rendering and file generation."""
    
    def __init__(self):
        self.logger = logging.getLogger(__name__)
    
    def render_template(self, template_path: str, data: Dict, 
                       output_path: str) -> bool:
        """
        Render template and write to output file.
        
        Args:
            template_path: Path to Jinja2 template
            data: Data to pass to template
            output_path: Path for output file
            
        Returns:
            True if successful, False otherwise
        """
        try:
            template_content = self._read_template(template_path)
            rendered_content = self._render_jinja_template(template_content, data)
            self._write_output(output_path, rendered_content)
            self.logger.info("Successfully generated: %s", output_path)
            return True
            
        except Exception as e:
            self.logger.error("Failed to render template: %s", e)
            return False
    
    def _read_template(self, template_path: str) -> str:
        """Read template file content."""
        template_file = Path(template_path)
        if not template_file.exists():
            raise TemplateRenderError(f"Template file not found: {template_path}")
        
        return template_file.read_text(encoding='utf-8')
    
    def _render_jinja_template(self, template_content: str, data: Dict) -> str:
        """Render Jinja2 template with provided data."""
        try:
            template = Template(template_content)
            return template.render(data)
        except Exception as e:
            raise TemplateRenderError(f"Template rendering failed: {e}")
    
    def _write_output(self, output_path: str, content: str) -> None:
        """Write content to output file, creating directories if needed."""
        output_file = Path(output_path)
        output_file.parent.mkdir(parents=True, exist_ok=True)
        output_file.write_text(content, encoding='utf-8')


class VLANGenerator:
    """Main class orchestrating the VLAN ID generation process."""
    
    def __init__(self, config_path: str, module_name: str, dry_run: bool = False):
        self.config_path = config_path
        self.module_name = module_name
        self.dry_run = dry_run
        self.logger = logging.getLogger(__name__)
        
        self.config = None
        self.scanner = CodeScanner()
        self.renderer = TemplateRenderer()
    
    def run(self) -> bool:
        """
        Execute the VLAN generation process.
        
        Returns:
            True if successful, False otherwise
        """
        try:
            self._load_configuration()
            self._validate_module()
            
            self.logger.info("Starting VLAN ID generation for module: %s", self.module_name)
            
            vlan_data = self.scanner.scan_module(self.config[self.module_name])
            
            if not vlan_data:
                self.logger.warning("No VLAN registrations found")
                return True
            
            if self.dry_run:
                self._display_dry_run_results(vlan_data)
                return True
            
            return self._generate_output_files(vlan_data)
            
        except Exception as e:
            self.logger.error("VLAN generation failed: %s", e)
            return False
    
    def _load_configuration(self) -> None:
        """Load and validate configuration."""
        self.config = ConfigLoader.load(self.config_path)
        self.logger.info("Configuration loaded successfully from: %s", self.config_path)
    
    def _validate_module(self) -> None:
        """Validate that the specified module exists in configuration."""
        if self.module_name not in self.config:
            raise ConfigurationError(
                f"Module '{self.module_name}' not found in configuration. "
                f"Available modules: {', '.join(self.config.keys())}"
            )
    
    def _display_dry_run_results(self, vlan_data: Dict[str, int]) -> None:
        """Display results without generating files (dry run mode)."""
        self.logger.info("DRY RUN - Would generate %d VLAN IDs:", len(vlan_data))
        for name, vlan_id in vlan_data.items():
            self.logger.info("  %s: %d", name, vlan_id)
    
    def _generate_output_files(self, vlan_data: Dict[str, int]) -> bool:
        """Generate output files using template."""
        module_config = self.config[self.module_name]
        
        # 使用更精确的时间格式，包含时分秒
        template_data = {
            'DATE': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            'VLANS': [{'NAME': name, 'ID': id} for name, id in vlan_data.items()]
        }
        
        success = self.renderer.render_template(
            module_config['template'],
            template_data,
            module_config['output']
        )
        
        if success:
            self.logger.info("Successfully generated VLAN IDs for %d VLANs", len(vlan_data))
        
        return success


def setup_logging(verbose: bool) -> None:
    """Configure logging based on verbosity level."""
    level = logging.DEBUG if verbose else logging.INFO
    logging.basicConfig(
        level=level,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        datefmt='%H:%M:%S'
    )


def parse_arguments() -> argparse.Namespace:
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description='Generate VLAN ID header files from source code scanning',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s                          # Use default config and vlan module
  %(prog)s -c config.json -m vlan   # Specify config and module
  %(prog)s --dry-run                # Scan without generating files
  %(prog)s --verbose                # Enable debug logging
        """
    )
    
    parser.add_argument(
        '--config', '-c',
        default='config.json',
        help='Path to JSON configuration file (default: config.json)'
    )
    
    parser.add_argument(
        '--module', '-m',
        default='vlan',
        help='Module to process (default: vlan)'
    )
    
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Scan without generating output files'
    )
    
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Enable verbose logging'
    )
    
    return parser.parse_args()


def main() -> int:
    """Main entry point for the VLAN generator script."""
    args = parse_arguments()
    setup_logging(args.verbose)
    
    logger = logging.getLogger(__name__)
    
    try:
        generator = VLANGenerator(args.config, args.module, args.dry_run)
        success = generator.run()
        
        return 0 if success else 1
        
    except KeyboardInterrupt:
        logger.info("Operation cancelled by user")
        return 130
    except Exception as e:
        logger.error("Fatal error: %s", e)
        return 1


if __name__ == '__main__':
    sys.exit(main())