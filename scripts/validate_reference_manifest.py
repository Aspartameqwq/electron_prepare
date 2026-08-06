#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""校验 docs/reference/REFERENCE_MANIFEST.yml。

检查项：
  1. YAML 可解析；
  2. 每个 reference 的 archive_path 存在；
  3. 每个 archive_path 的 SHA-256 与 archive_sha256 一致；
  4. source_url 为 null 或字符串。

用法： python scripts/validate_reference_manifest.py
退出码：0=通过；1=失败。
"""
import hashlib
import pathlib
import sys

MANIFEST = pathlib.Path("docs/reference/REFERENCE_MANIFEST.yml")
ROOT = pathlib.Path(".")

def main() -> int:
    if not MANIFEST.exists():
        print(f"FAIL: manifest not found: {MANIFEST}")
        return 1

    try:
        import yaml
    except ImportError:
        print("SKIP: PyYAML 未安装（pip install pyyaml 后可完整校验）；仅做存在性/哈希检查。")
        return check_files_no_yaml()

    data = yaml.safe_load(MANIFEST.read_text(encoding="utf-8"))
    if not isinstance(data, dict) or "references" not in data:
        print("FAIL: manifest 缺少 references 键或不是合法 YAML 映射")
        return 1

    fails = 0
    for ref in data["references"]:
        rid = ref.get("id", "?")
        # 1) archive_path 存在
        ap = ref.get("archive_path")
        if not ap:
            print(f"FAIL [{rid}]: 缺 archive_path")
            fails += 1
            continue
        p = ROOT / ap
        if not p.is_file():
            print(f"FAIL [{rid}]: 文件不存在 {ap}")
            fails += 1
            continue
        # 2) SHA-256 匹配
        want = ref.get("archive_sha256")
        if want:
            got = hashlib.sha256(p.read_bytes()).hexdigest()
            if got.lower() != want.lower():
                print(f"FAIL [{rid}]: SHA-256 不匹配 {ap}\n  want {want}\n  got  {got}")
                fails += 1
            else:
                print(f"OK   [{rid}]: {ap} (sha256 匹配)")
        else:
            print(f"WARN [{rid}]: 缺 archive_sha256（未校验哈希）")
        # 3) source_url 应为 null 或字符串
        su = ref.get("source_url")
        if su is not None and not isinstance(su, str):
            print(f"WARN [{rid}]: source_url 类型异常 {type(su)}")

    if fails:
        print(f"RESULT: FAILED ({fails})")
        return 1
    print("RESULT: ALL OK")
    return 0


def check_files_no_yaml() -> int:
    """无 PyYAML 时退化的存在性+哈希检查（按简单行解析 id/archive_path/sha）。"""
    import re
    fails = 0
    text = MANIFEST.read_text(encoding="utf-8")
    ids = re.findall(r"id:\s*(\S+)", text)
    paths = re.findall(r"archive_path:\s*\"([^\"]+)\"", text)
    shas = re.findall(r"archive_sha256:\s*\"([^\"]+)\"", text)
    if not (len(ids) == len(paths) == len(shas)):
        print("FAIL: 无法解析出等量的 id/archive_path/archive_sha256")
        return 1
    for rid, ap, want in zip(ids, paths, shas):
        p = ROOT / ap
        if not p.is_file():
            print(f"FAIL [{rid}]: 文件不存在 {ap}")
            fails += 1
            continue
        got = hashlib.sha256(p.read_bytes()).hexdigest()
        if got.lower() != want.lower():
            print(f"FAIL [{rid}]: SHA-256 不匹配 {ap}")
            fails += 1
        else:
            print(f"OK   [{rid}]: sha256 匹配")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
