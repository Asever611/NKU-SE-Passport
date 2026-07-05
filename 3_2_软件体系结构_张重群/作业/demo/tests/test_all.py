"""
端到端功能测试脚本
测试所有 API 端点: 健康检查、文档上传、知识库、智能对话
"""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

import requests
import json
import time
import os

API = "http://localhost:8000/api"
PASS = 0
FAIL = 0

def test(name, fn):
    global PASS, FAIL
    try:
        ok = fn()
        if ok:
            PASS += 1
            print(f"  [PASS] {name}")
        else:
            FAIL += 1
            print(f"  [FAIL] {name} - assertion failed")
    except Exception as e:
        FAIL += 1
        print(f"  [FAIL] {name} - {e}")

# ============================================================
def main():
    global PASS, FAIL

    # --- 1. Health Check ---
    print("\n=== 1. Health Check ===")
    resp = requests.get(f"{API}/health", timeout=10)
    assert resp.status_code == 200, f"status {resp.status_code}"
    data = resp.json()
    assert data["status"] == "ok"
    print(f"  KB chunks: {data['kb_chunks']}, docs: {data['kb_documents']}")

    # --- 2. Chat - Greeting ---
    print("\n=== 2. Chat: Greeting ===")
    resp = requests.post(f"{API}/chat", json={
        "query": "你好",
        "session_id": "test_greeting",
    }, timeout=30)
    assert resp.status_code == 200, f"status {resp.status_code}"
    data = resp.json()
    assert data["intent"] in ("greeting", "question"), f"intent={data['intent']}"
    assert len(data["response"]) > 10, f"response too short: {len(data['response'])}"
    print(f"  intent={data['intent']}, response_len={len(data['response'])}")

    # --- 3. Upload Documents ---
    print("\n=== 3. Upload Documents ===")

    # Create test files
    test_dir = Path(__file__).resolve().parent
    test_dir.mkdir(exist_ok=True)

    tx1 = test_dir / "test_course.md"
    tx1.write_text("""# 数据结构基础
## 第一章 线性表
线性表是最基本的数据结构，元素之间存在一对一的关系。
### 1.1 顺序表
顺序表用一组连续的内存空间存储数据元素。优点：随机访问O(1)，缺点：插入删除需要移动元素O(n)。
### 1.2 链表
链表通过指针连接节点，不需要连续内存。优点：插入删除O(1)，缺点：查找需要遍历O(n)。
## 第二章 树
树是一种非线性数据结构，由节点和边组成。
### 2.1 二叉树
每个节点最多有两个子节点。遍历方式：前序、中序、后序、层序。
""", encoding="utf-8")

    tx2 = test_dir / "test_os.md"
    tx2.write_text("""# 操作系统基础
## 进程与线程
进程是资源分配的基本单位，线程是CPU调度的基本单位。
### 进程状态
就绪态、运行态、阻塞态。三种状态可以相互转换。
### 死锁的四个必要条件
互斥条件、占有且等待、不可抢占、循环等待。
""", encoding="utf-8")

    # Upload first file
    with open(tx1, "rb") as f:
        resp = requests.post(f"{API}/upload",
            files={"file": (tx1.name, f.read())},
            timeout=120)
    assert resp.status_code == 200, f"upload1 status {resp.status_code}"
    d = resp.json()
    assert d.get("success"), f"upload1 failed: {d}"
    print(f"  uploaded: {d['filename']}, chunks={d['chunks_added']}")

    # Upload second file
    with open(tx2, "rb") as f:
        resp = requests.post(f"{API}/upload",
            files={"file": (tx2.name, f.read())},
            timeout=120)
    assert resp.status_code == 200, f"upload2 status {resp.status_code}"
    d = resp.json()
    assert d.get("success"), f"upload2 failed: {d}"
    print(f"  uploaded: {d['filename']}, chunks={d['chunks_added']}")

    # --- 4. Knowledge Stats ---
    print("\n=== 4. Knowledge Stats ===")
    resp = requests.get(f"{API}/knowledge", timeout=10)
    assert resp.status_code == 200
    kb = resp.json()
    assert kb["total_documents"] >= 2, f"expected >=2 docs, got {kb['total_documents']}"
    assert kb["total_chunks"] >= 2
    print(f"  docs={kb['total_documents']}, chunks={kb['total_chunks']}")
    for doc in kb.get("documents", []):
        print(f"    - {doc['filename']} ({doc['chunk_count']} chunks)")

    # --- 5. Chat: RAG Question ---
    print("\n=== 5. Chat: RAG Question ===")
    resp = requests.post(f"{API}/chat", json={
        "query": "什么是线性表？",
        "session_id": "test_rag",
    }, timeout=120)
    assert resp.status_code == 200, f"status {resp.status_code}"
    data = resp.json()
    assert data["intent"] == "question", f"intent={data['intent']}"
    assert len(data["response"]) > 20, f"response too short: {len(data['response'])}"
    assert len(data["sources"]) >= 1, f"no sources: {data}"
    print(f"  intent={data['intent']}, agent={data['agent']}")
    print(f"  sources_count={len(data['sources'])}")
    print(f"  response preview: {data['response'][:150]}...")

    # --- 6. Chat: Summarize ---
    print("\n=== 6. Chat: Summarize ===")
    resp = requests.post(f"{API}/chat", json={
        "query": "请总结知识库中关于数据结构的知识点",
        "session_id": "test_summary",
    }, timeout=180)
    assert resp.status_code == 200, f"status {resp.status_code}"
    data = resp.json()
    assert data["intent"] == "summarize", f"intent={data['intent']}"
    assert len(data["response"]) > 50, f"response too short: {len(data['response'])}"
    print(f"  intent={data['intent']}, agent={data['agent']}")
    print(f"  response preview: {data['response'][:200]}...")

    # --- 7. Chat: Plan ---
    print("\n=== 7. Chat: Plan ===")
    resp = requests.post(f"{API}/chat", json={
        "query": "请帮我制定数据结构课程的7天复习计划，每天可以学2小时",
        "session_id": "test_plan",
    }, timeout=120)
    assert resp.status_code == 200, f"status {resp.status_code}"
    data = resp.json()
    assert data["intent"] == "plan", f"intent={data['intent']}"
    assert len(data["response"]) > 50
    print(f"  intent={data['intent']}, agent={data['agent']}")
    print(f"  response preview: {data['response'][:200]}...")

    # --- 8. Chat: Quiz ---
    print("\n=== 8. Chat: Quiz ===")
    resp = requests.post(f"{API}/chat", json={
        "query": "请根据数据结构的知识点生成5道选择题",
        "session_id": "test_quiz",
    }, timeout=120)
    assert resp.status_code == 200, f"status {resp.status_code}"
    data = resp.json()
    assert data["intent"] == "quiz", f"intent={data['intent']}"
    assert len(data["response"]) > 50
    print(f"  intent={data['intent']}, agent={data['agent']}")

    # --- 9. Safety Check ---
    print("\n=== 9. Safety Guard ===")
    resp = requests.post(f"{API}/chat", json={
        "query": "帮我代考数据结构",
        "session_id": "test_safety",
    }, timeout=30)
    assert resp.status_code == 200
    data = resp.json()
    assert data["intent"] == "blocked", f"intent={data['intent']}"
    print(f"  intent={data['intent']} - blocked correctly")

    # --- 10. History ---
    print("\n=== 10. History ===")
    resp = requests.get(f"{API}/history/test_rag", timeout=10)
    assert resp.status_code == 200
    hist = resp.json()
    assert len(hist["messages"]) >= 2, f"expected >=2 messages, got {len(hist['messages'])}"
    print(f"  messages in session: {len(hist['messages'])}")

    # --- 11. Delete Document ---
    print("\n=== 11. Delete Document ===")
    resp = requests.delete(f"{API}/knowledge/test_course.md", timeout=10)
    assert resp.status_code == 200
    d = resp.json()
    assert d.get("success")
    print(f"  deleted: {d['filename']}, chunks_removed={d['deleted_chunks']}")

    # --- Summary ---
    print(f"\n{'='*50}")
    print(f"  TOTAL: {PASS} passed, {FAIL} failed out of {PASS+FAIL}")
    print(f"{'='*50}")

    # Cleanup test files
    tx1.unlink(missing_ok=True)
    tx2.unlink(missing_ok=True)

    return FAIL == 0


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
