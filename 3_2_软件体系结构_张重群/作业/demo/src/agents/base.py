"""
Agent 抽象基类
定义所有专业Agent的统一接口, 遵循策略模式
"""

from abc import ABC, abstractmethod
from src.core.llm_service import llm_service


class BaseAgent(ABC):
    """
    智能体抽象基类
    所有专业Agent必须继承此类并实现 process() 方法
    """

    def __init__(self, name: str, description: str = ""):
        self.name = name
        self.description = description
        self.llm = llm_service

    @abstractmethod
    def process(self, input_data: dict) -> dict:
        """
        处理任务 (核心方法)
        Args:
            input_data: Agent输入数据
        Returns:
            Agent输出结果
        """
        pass

    def validate(self, output: dict) -> bool:
        """验证输出是否合法 (子类可覆写)"""
        return output is not None and isinstance(output, dict)

    def __repr__(self):
        return f"{self.name}({self.description})"
