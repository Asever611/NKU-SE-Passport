"""
RetrievalAgent (检索智能体)
职责: 智能问答 — 以LLM自身知识为基础，知识库内容作为增强补充
应用设计模式: RAG (知识检索增强生成)

核心原则:
- LLM 本身具备海量知识，始终回答问题
- 知识库是"增强层"而非"唯一数据源"
- 有课件内容 → RAG增强，标注引用来源
- 无课件内容 → 基于LLM自身知识回答，标注"基于通用知识"
"""

from src.agents.base import BaseAgent
from src.core.knowledge_base import knowledge_base


class RetrievalAgent(BaseAgent):
    """智能问答 Agent — 知识库增强，非限制"""

    def __init__(self):
        super().__init__("RetrievalAgent", "智能问答 (RAG增强)")

    def process(self, input_data: dict) -> dict:
        query = input_data.get("query", "")
        history = input_data.get("history", [])

        # 检索知识库
        search_results = knowledge_base.search(query, top_k=5)

        # 构建系统提示 — 始终以LLM知识为基础
        if search_results:
            context_parts = []
            sources = []
            for i, result in enumerate(search_results):
                source = result.get("metadata", {}).get("source", "未知来源")
                content = result.get("content", "")
                score = result.get("score", 0)
                context_parts.append(
                    f"[课件资料{i+1}] 来源: {source} (相关度: {score:.2f})\n{content}"
                )
                if source not in [s["source"] for s in sources]:
                    sources.append({"source": source, "relevance": round(score, 3)})

            context = "\n\n---\n\n".join(context_parts)
            system_prompt = (
                "你是一位专业的学习辅导助手。请回答用户的问题。\n\n"
                "以下是从用户课件中找到的相关资料，可作为**补充参考**:\n"
                f"{context}\n\n"
                "回答要求:\n"
                "1. 综合运用你的知识和课件资料，给出完整准确的回答\n"
                "2. 当使用课件资料中的内容时，标注 [课件参考]\n"
                "3. 使用中文，条理清晰，要点层次化\n"
                "4. 如果课件资料未覆盖某方面，用你的知识补充并标注 (通用知识)"
            )
        else:
            sources = []
            system_prompt = (
                "你是一位专业的学习辅导助手。请回答用户的问题。\n\n"
                "注意: 知识库中暂无与该问题直接相关的课件，但请基于你的知识给出完整回答。\n\n"
                "回答要求:\n"
                "1. 使用中文，条理清晰，鼓励用要点列表和层次结构\n"
                "2. 末尾加一行提示: '💡 提示: 上传相关课件可让我给出更贴合课程内容的回答'"
            )

        messages = [{"role": "system", "content": system_prompt}]
        for msg in history[-6:]:
            messages.append({"role": msg.get("role", "user"), "content": msg.get("content", "")})
        messages.append({"role": "user", "content": query})

        response = self.llm.chat_with_retry(messages, temperature=0.5, max_tokens=4096)

        return {"response": response, "sources": sources}
