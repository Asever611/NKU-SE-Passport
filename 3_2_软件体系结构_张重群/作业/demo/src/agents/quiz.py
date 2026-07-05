"""
QuizAgent (测验智能体)
职责: 自动生成练习题并附答案解析，知识库内容用于让题目更贴合课件
应用设计模式: 知识检索增强 + Planning

核心原则:
- LLM 自身具备出题能力，始终生成题目
- 知识库用于让题目更精准地覆盖用户课件内容
"""

from src.agents.base import BaseAgent
from src.core.knowledge_base import knowledge_base


class QuizAgent(BaseAgent):
    """自测练习智能体 — 知识库增强题目针对性"""

    def __init__(self):
        super().__init__("QuizAgent", "自测题目生成")

    def process(self, input_data: dict) -> dict:
        query = input_data.get("query", "")
        history = input_data.get("history", [])

        # 检索相关知识库内容作为出题素材
        search_results = knowledge_base.search(query, top_k=5)
        if search_results:
            context = "\n\n".join([r.get("content", "") for r in search_results][:3])
            context_note = "请优先基于以上课件内容出题。"
        else:
            # 没有课件也要出题，用LLM自身知识
            all_chunks = knowledge_base.get_all_chunks(max_chars=2000)
            if all_chunks:
                context = all_chunks
                context_note = "以下为知识库全部内容，请尽量围绕这些知识出题。如内容不足，可用你的知识补充。"
            else:
                context = ""
                context_note = ("知识库中暂无课件。请基于你的知识出题，"
                                "末尾提醒用户上传课件可使题目更贴合课程。")

        # 判断题目偏好
        query_lower = query.lower()
        if "选择" in query_lower or "多选" in query_lower:
            type_instruction = (
                "生成5道单项选择题，每道4选项(A/B/C/D)，标注正确答案和详细解析。"
            )
        elif "简答" in query_lower or "问答" in query_lower:
            type_instruction = "生成3道简答题，每道附参考答案要点(3-5条)。"
        else:
            type_instruction = "生成5道单项选择题 + 1道综合简答题。"

        system_prompt = (
            "你是一位严格的出题老师。请根据用户需求生成高质量的练习题。\n\n"
            "出题原则:\n"
            "1. 覆盖核心知识点，难度梯度合理(基础→中等→综合)\n"
            "2. 选择题干扰项要有迷惑性但非错误\n"
            "3. 每道题附带详细答案解析\n"
            "4. 使用Markdown格式，结构清晰\n"
            "5. 最后添加'📊 自评建议': 告诉学生各题考察的知识点和复习建议\n\n"
            f"{type_instruction}"
        )

        messages = [
            {"role": "system", "content": system_prompt},
            {"role": "user",
             "content": f"课件参考内容:\n{context[:3000]}\n\n{context_note}\n\n用户需求: {query}\n\n请生成练习题:"},
        ]

        response = self.llm.chat_with_retry(messages, temperature=0.6, max_tokens=4096)

        return {"response": response, "sources": []}
