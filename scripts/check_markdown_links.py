#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Markdown 本地链接检查（P0 门禁）。

对仓库内 .md 的本地链接 `[text](path)`（非 http/https、非锚点）：
  - 相对文件路径：以链接所在文件目录解析，存在即可；
  - 仓库根相对路径：以仓库根解析。
不检查 http(s) 外链、`#锚点`、图片。
用法： python scripts/check_markdown_links.py
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
LINK_RE = re.compile(r"\[[^\]]*\]\(([^)]+)\)")
SKIP_DIRS = {".git", "examples_and_documents"}   # 第三方参考库文件不参与链接检查（其内部链接按原始工程自洽）
SKIP_EXTS = {".jpg", ".png", ".gif"}


def main() -> int:
    fails = 0
    for md in ROOT.rglob("*.md"):
        if md.is_symlink() or md.name.startswith("."):
            continue
        rel_parts = md.relative_to(ROOT).parts
        if any(rel_parts[0] == s for s in SKIP_DIRS):
            continue
        base = md.parent
        for m in LINK_RE.finditer(md.read_text(encoding="utf-8")):
            target = m.group(1).strip()
            if target.startswith(("http://", "https://", "#", "mailto:")):
                continue
            # 去掉锚点与查询
            path_part = target.split("#")[0].split("?")[0]
            if not path_part:
                continue
            if path_part.lower().endswith(tuple(SKIP_EXTS)):
                continue
            p = (base / path_part).resolve()
            if not p.exists():
                # 也尝试以仓库根解析（部分文档用根相对路径）
                p2 = (ROOT / path_part).resolve()
                if not p2.exists():
                    print(f"FAIL: {md.relative_to(ROOT)}: 链接不存在 -> {target}")
                    fails += 1
    if fails:
        print(f"RESULT: FAILED ({fails})")
        return 1
    print("RESULT: ALL OK (markdown 本地链接)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
