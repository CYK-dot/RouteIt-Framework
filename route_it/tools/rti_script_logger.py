#!/usr/bin/env python3
"""
RTI Script Logger - 统一的日志输出模块
提供CMake风格的日志输出格式
"""

import sys
from datetime import datetime
from enum import Enum
from typing import Any

class LogLevel(Enum):
    DEBUG = 0
    INFO = 1
    NOTICE = 2
    WARNING = 3
    ERROR = 4
    FATAL = 5

class RTIScriptLogger:
    """RTI脚本日志记录器"""
    
    # CMake风格的颜色代码
    COLORS = {
        'RED': '\033[1;31m',
        'GREEN': '\033[1;32m', 
        'YELLOW': '\033[1;33m',
        'BLUE': '\033[1;34m',
        'MAGENTA': '\033[1;35m',
        'CYAN': '\033[1;36m',
        'WHITE': '\033[1;37m',
        'RESET': '\033[0m'
    }
    
    # 日志级别对应的颜色和前缀
    LEVEL_CONFIG = {
        LogLevel.DEBUG: ('CYAN', '-- '),
        LogLevel.INFO: ('GREEN', '-- '),
        LogLevel.NOTICE: ('BLUE', '-- '),
        LogLevel.WARNING: ('YELLOW', '-- '),
        LogLevel.ERROR: ('RED', '-- '),
        LogLevel.FATAL: ('RED', '** ')
    }
    
    def __init__(self, min_level: LogLevel = LogLevel.INFO):
        self.min_level = min_level
    
    def _log(self, level: LogLevel, message: str, *args, **kwargs):
        """内部日志方法"""
        if level.value < self.min_level.value:
            return
            
        color_name, prefix = self.LEVEL_CONFIG[level]
        color_code = self.COLORS[color_name]
        reset_code = self.COLORS['RESET']
        
        formatted_message = message
        if args or kwargs:
            try:
                formatted_message = message.format(*args, **kwargs)
            except Exception:
                formatted_message = message
        
        timestamp = datetime.now().strftime("%H:%M:%S")
        full_message = f"{color_code}{prefix}[{timestamp}] {formatted_message}{reset_code}"
        
        # 错误和致命信息输出到stderr
        if level in [LogLevel.ERROR, LogLevel.FATAL]:
            print(full_message, file=sys.stderr)
        else:
            print(full_message)
    
    def debug(self, message: str, *args, **kwargs):
        """调试信息"""
        self._log(LogLevel.DEBUG, message, *args, **kwargs)
    
    def info(self, message: str, *args, **kwargs):
        """一般信息"""
        self._log(LogLevel.INFO, message, *args, **kwargs)
    
    def notice(self, message: str, *args, **kwargs):
        """通知信息"""
        self._log(LogLevel.NOTICE, message, *args, **kwargs)
    
    def warning(self, message: str, *args, **kwargs):
        """警告信息"""
        self._log(LogLevel.WARNING, message, *args, **kwargs)
    
    def error(self, message: str, *args, **kwargs):
        """错误信息"""
        self._log(LogLevel.ERROR, message, *args, **kwargs)
    
    def fatal(self, message: str, *args, **kwargs):
        """致命错误信息"""
        self._log(LogLevel.FATAL, message, *args, **kwargs)
        sys.exit(1)

# 创建全局日志实例
logger = RTIScriptLogger()

# 导出便捷函数
def debug(msg: str, *args, **kwargs): logger.debug(msg, *args, **kwargs)
def info(msg: str, *args, **kwargs): logger.info(msg, *args, **kwargs)
def notice(msg: str, *args, **kwargs): logger.notice(msg, *args, **kwargs)
def warning(msg: str, *args, **kwargs): logger.warning(msg, *args, **kwargs)
def error(msg: str, *args, **kwargs): logger.error(msg, *args, **kwargs)
def fatal(msg: str, *args, **kwargs): logger.fatal(msg, *args, **kwargs)

def set_log_level(level: LogLevel):
    """设置日志级别"""
    logger.min_level = level

if __name__ == "__main__":
    # 测试代码
    debug("这是一条调试信息")
    info("这是一条信息")
    notice("这是一条通知")
    warning("这是一条警告")
    error("这是一条错误")
    # fatal("这是一条致命错误")  # 会退出程序