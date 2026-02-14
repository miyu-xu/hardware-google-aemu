# Human Engineer's Guide to Agentic Development

Welcome to the future of development in the Android Emulator codebase. This
document explains how to use the "Multi-Agent" system in this repository.

## Getting Started: Initialization

If you typically start `gemini-cli` from the source root, you must first link
the AEMU agentic workflow to your root directory:

1.  Navigate to the source root.
2.  Run the setup script:
    ```bash
    python3 hardware/google/aemu/setup_agents.py
    ```

This script creates symbolic links (`AGENTS.md` and `.skills/`) at your source
root so that every Gemini session automatically "hires" your specialized AEMU
engineering team.

## 0. Your Role: The Director & Judge

In this system, you are the **Director of Engineering**. Your role is
critical to the success of the agents:

*   **Define the "What":** You provide the high-level objective and context.
*   **Approve the "Why":** You provide final approval for the `DESIGN.md`. The
    **Critic** will have already performed an architectural audit; your role is
    to ensure the final strategy aligns with your vision.
*   **Audit the "How":** While the Critic and Style Enforcer catch technical
    errors, you ensure the solution fits the team's long-term goals.
*   **Calibrate the System:** When an agent fails, you tell the **Overseer** to
    update instructions so it never makes that mistake again.

## 0.1 Fluid Authority: Your Interaction Modes

Your role shifts based on the granularity of your request:

*   **Strategic (CEO Mode):** You are building the team.
    *   *Example:* "Overseer, we are seeing too many race conditions. Update the
        Critic to be more aggressive about checking Mutex usage."
*   **Tactical (Director Mode):** You are delivering features.
    *   *Example:* "Implement the gRPC service defined in `emulator.proto`. Use
        the Planner to draft the service architecture first."
*   **Operational (Peer Mode):** You are working in the trenches.
    *   *Example:* "Write a unit test for this function" or "Format this file."

The agents remain consistent specialists regardless of your mode. Even in "Peer
Mode," the **Critic** will still warn you if you violate a safety standard.

## 0.2 Autonomy Tiers: Managing the "Leash"

You can control how often we interrupt you by "promoting" the team. Ask the
**Overseer** to change tiers:

*   **Tier 1: Discovery** - Auto-approves `ls`, `grep`, `find`, `git status`.
*   **Tier 2: Verification** - Adds `bazel build`, `bazel test`, and `cat`.
*   **Tier 3: Surgical** - Adds `write_file` and `replace`.
*   **Tier 4: Full Autonomy** - Full shell and `git` access.

> **User:** "Overseer, promote the team to Tier 3 so you can implement the
> feature without asking for every file edit."

---

## 1. What is a "Multi-Agent" Workflow?

You do **not** need to open multiple terminal windows or spawn new instances of
Gemini CLI. Think of Gemini CLI as a **Senior Engineer** who can swap "hats"
(personas). When you ask me to "activate a skill," I am calling over a
specialist to handle a specific part of the task.

**The Workflow:**
1.  You give me a high-level task.
2.  I use specialized skills (agents) to execute phases (Planning, Reviewing).
3.  You provide feedback to the **Overseer** to refine agent behavior.

---

## 2. How to Use the Agents

### A. The Direct Request (The Most Common Way)
You don't usually need to manually activate skills. Just tell me what you want.

> **User:** "I need to add a feature to SocketUtils. Write a design doc first."

### B. Explicit Skill Activation
If you want to focus my attention on a specific standard, tell me to activate a
skill.

> **User:** "Activate the critic skill and review my changes for race
> conditions."

### C. The Feedback Loop (The Friction Log)
The **Overseer** is always watching. Every time you:
*   Correct a command.
*   Point us to a missing file.
*   Clarify a requirement.

The Overseer logs this as **"Friction"** and handles it at the end of the
session.

### D. Expanding the Team (New Agent Requests)
If you notice a recurring task that doesn't fit a role, ask the **Overseer** to
hire a new specialist.

> **User:** "Overseer, create a 'Protobuf Specialist' agent that knows our
> naming conventions for .proto files."

### E. The Autonomy Audit (The Zero-Repeat Goal)
The **Overseer** performs an **Autonomy Audit** after every task:
1.  **Analyze the Log:** The Overseer looks at every time you intervened.
2.  **Hardening:** It updates the specialist agents to remember corrections.
3.  **Evolution:** It refines the workflow in `AGENTS.md`.

### F. Knowledge Handover (The "Director's Briefing")
The **Documenter** agent is responsible for a final "Handover":
1.  **Code Comments:** Ensuring the new code explains itself.
2.  **Architecture Sync:** Updating `ARCHITECTURE.md`.
3.  **The Briefing:** Providing a concise summary of the changes.

---

## 3. Meet the Team

| Agent | When to call them |
| :--- | :--- |
| **Planner** | "Let's plan out this new feature." |
| **Critic** | "Review this code for memory safety." |
| **Debugger** | "I'm stuck on a crash or need to see data flow." |
| **Style Enforcer** | "Format this file and check naming conventions." |
| **Documenter** | "Update the ARCHITECTURE.md for these changes." |
| **License Agent** | "Ensure correct Apache/GPL headers." |
| **Committer** | "Prepare a commit message for Gerrit." |
| **Overseer** | "Update the agent's instructions." |

---

## 4. Tips for Success

1.  **Trust the Design Phase:** Don't skip the `planner`. It ensures the
    architecture is right before writing code.
2.  **Embrace the Living Plan:** Expect subtasks to change as we learn more.
3.  **Be Specific with the Overseer:** The more specific your feedback, the
    better the agents become.
4.  **Review the Agent Files:** You can read the files in `.skills/` to see the
    instructions the agents follow.

## 5. Why are we doing this?

The goal is to reduce **your** mental overhead. You remain the **Director**,
focusing on high-level logic, while the agents handle the "grunt work" of
engineering excellence.
