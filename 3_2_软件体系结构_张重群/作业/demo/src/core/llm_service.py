"""
LLM 服务统一封装
支持 DeepSeek API (兼容 OpenAI 格式)，提供统一的大模型调用接口
"""

import os
from openai import OpenAI
from src.core.config import config


class LLMService:
    """
    LLM 调用统一封装
    使用 OpenAI 兼容客户端，支持 DeepSeek 等模型
    """

    def __init__(self):
        self.client = OpenAI(
            api_key=config.LLM_API_KEY,
            base_url=config.LLM_API_BASE,
        )
        self.model = config.LLM_MODEL
        self._embedding_model = None

    def chat(
        self,
        messages: list[dict],
        temperature: float = 0.7,
        max_tokens: int = 4096,
        stream: bool = False,
    ) -> str:
        """
        发送聊天请求
        Args:
            messages: [{"role": "system"|"user"|"assistant", "content": "..."}]
            temperature: 创造性参数 (0-2)
            max_tokens: 最大输出 token 数
            stream: 是否流式输出
        Returns:
            LLM 回复文本
        """
        try:
            response = self.client.chat.completions.create(
                model=self.model,
                messages=messages,
                temperature=temperature,
                max_tokens=max_tokens,
                stream=stream,
            )
            if stream:
                return response  # 返回流对象
            return response.choices[0].message.content
        except Exception as e:
            return f"[LLM Error] {str(e)}"

    def chat_with_retry(self, messages: list[dict], max_retries: int = 2, **kwargs) -> str:
        """带重试的聊天请求"""
        for attempt in range(max_retries + 1):
            result = self.chat(messages, **kwargs)
            if not result.startswith("[LLM Error]"):
                return result
            if attempt < max_retries:
                import time
                time.sleep(1 * (attempt + 1))
        return result

    def _get_embedding_model(self):
        """懒加载嵌入模型 (避免重复加载)"""
        if self._embedding_model is None:
            from sentence_transformers import SentenceTransformer
            # 设置 HuggingFace 镜像 (国内加速)
            if config.HF_ENDPOINT:
                os.environ.setdefault("HF_ENDPOINT", config.HF_ENDPOINT)
            self._embedding_model = SentenceTransformer(config.EMBEDDING_MODEL)
        return self._embedding_model

    def generate_embedding(self, text: str) -> list[float]:
        """生成文本向量 (使用本地 Sentence Transformer 模型)"""
        model = self._get_embedding_model()
        embedding = model.encode(text, normalize_embeddings=True)
        return embedding.tolist()


# 全局单例
llm_service = LLMService()
