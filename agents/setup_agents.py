#!/usr/bin/env python3
from pathlib import Path
import shutil
import json
import sys


def setup():
    # 1. Path Calculation
    script_dir = Path(__file__).resolve().parent
    source_root = script_dir.parents[3]

    print("-" * 50)
    print(f"Setting up AEMU Agents at: {source_root}")
    print("-" * 50)

    # 2. Environment Detection
    print("Detecting build environment...")
    build_tools = []
    if (source_root / "MODULE.bazel").exists() or \
       (source_root / "WORKSPACE").exists():
        print("-> Detected Bazel Environment")
        build_tools.append("bazel")

    if (source_root / "CMakeLists.txt").exists() or \
       (source_root / "rebuild.sh").exists():
        print("-> Detected CMake/Python Environment")
        build_tools.extend(["cmake", "make", "python3", "python"])

    # 3. Prompt for Autonomy Level
    print("\nChoose an Autonomy Level for the Agents:")
    print("1) Minimal    - [minimal_discovery.toml] Read-only discovery.")
    print("2) Standard   - [standard_verification.toml] Adds build/test tools. [RECOMMENDED]")
    print("3) Advanced   - [advanced_surgical.json] Adds autonomous file editing.")
    print("4) Maximum    - [maximum_autonomy.json] Full shell/git access.")

    try:
        choice = input("Select level (1-4) [2]: ").strip() or "2"
    except EOFError:
        choice = "2"

    tier_map = {
        "1": "minimal_discovery.toml",
        "2": "standard_verification.toml",
        "3": "advanced_surgical.toml",
        "4": "maximum_autonomy.toml"
    }
    tier_file = tier_map.get(choice, "standard_verification.toml")
    tier_path = script_dir / ".gemini/policies" / tier_file

    # Create .gemini directory
    gemini_dir = source_root / ".gemini"
    gemini_dir.mkdir(exist_ok=True)

    # 4. Generate adaptive policy.toml
    policies_dir = gemini_dir / "policies"
    policies_dir.mkdir(exist_ok=True)

    policy_dest = policies_dir / "aemu_policy.toml"

    with open(tier_path, 'r') as f:
        policy_content = f.read()

    # Inject detected build tools if Tier >= 2
    if choice in ["2", "3", "4"]:
        for tool in build_tools:
            if f'commandPrefix = "{tool}"' not in policy_content:
                policy_content += (
                    f'\n[[rule]]\ntoolName = "run_shell_command"\n'
                    f'commandPrefix = "{tool}"\ndecision = "allow"\n'
                    f'priority = 100\n'
                )

    with open(policy_dest, 'w') as f:
        f.write(policy_content)

    # 5. Create symlinks
    def safe_link(target_rel, link_name, base_dir=source_root):
        full_link_path = base_dir / link_name
        if full_link_path.is_symlink():
            full_link_path.unlink()
        elif full_link_path.exists():
            print(f"ERROR: Local {link_name} already exists as a real file/dir at {full_link_path}")
            print("Please backup and remove it manually before running this script.")
            return False

        try:
            full_link_path.symlink_to(target_rel)
            return True
        except OSError as e:
            print(f"ERROR: Failed to create symlink {link_name}: {e}")
            return False

    if not safe_link("hardware/google/aemu/agents/AGENTS.md", "AGENTS.md"):
        sys.exit(1)
    if not safe_link("hardware/google/aemu/agents/.skills", ".skills"):
        sys.exit(1)

    # Create skills directory in .gemini
    gemini_skills_dir = gemini_dir / "skills"
    if gemini_skills_dir.is_symlink():
        print(f"Removing existing symlink for skills directory: {gemini_skills_dir}")
        gemini_skills_dir.unlink()
    gemini_skills_dir.mkdir(exist_ok=True)

    skills_source = script_dir / ".skills"
    for skill_path in skills_source.iterdir():
        if skill_path.is_dir():
            if not safe_link(Path("../../hardware/google/aemu/agents/.skills") / skill_path.name,
                             skill_path.name, base_dir=gemini_skills_dir):
                print(f"Warning: Could not link skill {skill_path.name} into .gemini/skills.")

    # Also link the tiers/policies folder so they are visible
    if not safe_link("../hardware/google/aemu/agents/.gemini/policies",
                     "policies_templates", base_dir=gemini_dir):
        print("Warning: Could not link policies directory.")

    print("-" * 50)
    print(f"Done. Gemini CLI configured with {tier_file}.")
    print(f"IMPORTANT: Trust the root folder: gemini-cli trust {source_root}")
    print("-" * 50)


if __name__ == "__main__":
    setup()
