"""快速端到端测试 — 验证核心功能链"""
import sys, io, requests, json
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

API = "http://localhost:8000/api"
p = 0; f = 0
def t(name, ok):
    global p, f
    if ok: p += 1; print(f"  [PASS] {name}")
    else: f += 1; print(f"  [FAIL] {name}")

# 1. Health
r = requests.get(f"{API}/health"); t("Health", r.json()["status"]=="ok")

# 2. Greeting
r = requests.post(f"{API}/chat", json={"query":"你好","session_id":"s1"})
t("Greeting", r.json()["intent"]=="greeting")

# 3. Upload
data = "# 计算机网络\n## TCP三次握手\nSYN→SYN-ACK→ACK建立连接。\n## HTTP协议\n无状态应用层协议。"
import tempfile, os
tmp = tempfile.NamedTemporaryFile(mode='w', suffix='.md', delete=False, encoding='utf-8')
tmp.write(data); tmp.close()
with open(tmp.name, 'rb') as fh:
    r = requests.post(f"{API}/upload", files={"file": (os.path.basename(tmp.name), fh.read())}, timeout=60)
t("Upload", r.json().get("success") and r.json()["chunks_added"] > 0)
os.unlink(tmp.name)

# 4. KB Stats
r = requests.get(f"{API}/knowledge")
t("KB Stats", r.json()["total_documents"] >= 1)

# 5. RAG Question
r = requests.post(f"{API}/chat", json={"query":"TCP三次握手的过程是什么？","session_id":"s2"}, timeout=120)
d = r.json()
t("RAG Question", d["intent"]=="question" and d["agent"]=="RetrievalAgent" and len(d["sources"])>=1)
print(f"    Response preview: {d['response'][:120]}...")

# 6. Summarize
r = requests.post(f"{API}/chat", json={"query":"总结一下计算机网络的核心知识点","session_id":"s3"}, timeout=180)
d = r.json()
t("Summarize", d["intent"]=="summarize" and d["agent"]=="SummarizerAgent" and len(d["response"])>30)
print(f"    Response preview: {d['response'][:120]}...")

# 7. Plan
r = requests.post(f"{API}/chat", json={"query":"帮我制定计算机网络3天复习计划","session_id":"s4"}, timeout=120)
d = r.json()
t("Plan", d["intent"]=="plan" and d["agent"]=="PlannerAgent")

# 8. Quiz
r = requests.post(f"{API}/chat", json={"query":"生成3道关于计算机网络的题目","session_id":"s5"}, timeout=120)
d = r.json()
t("Quiz", d["intent"]=="quiz" and d["agent"]=="QuizAgent")

# 9. Safety
r = requests.post(f"{API}/chat", json={"query":"帮我代考","session_id":"s6"})
t("Safety", r.json()["intent"]=="blocked")

# 10. History
r = requests.get(f"{API}/history/s2")
t("History", len(r.json().get("messages",[])) >= 2)

# Done
print(f"\n{'='*40}")
print(f"Results: {p} passed, {f} failed, {p+f} total")
