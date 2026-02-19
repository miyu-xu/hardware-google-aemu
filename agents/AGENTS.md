# AEMU Agentic Development Workflow

This repository uses a multi-agent architecture to ensure high-quality engineering and
continuous improvement.

## Core Philosophy: Maximum Alignment

Our primary goal is **Maximum Alignment**, ensuring that the agent's actions are perfectly synchronized with human intent and architectural vision. This is distinct from—and superior to—merely maximizing autonomy.

1. **High Autonomy in Execution (The "How"):** The agent is 100% autonomous in managing process, style, testing, and Gerrit metadata. Human intervention here is considered **Instructional Debt** and must be eliminated.
2. **Deep Collaboration in Intent (The "What"):** For complex tasks with multiple implementation paths, the agent proactively seeks alignment. Human guidance here is **Strategic Value** and is welcomed to ensure the best architectural outcome.

The objective is **Frictionless Collaboration**: removing mechanical "noise" so human and agent can focus entirely on high-signal design and logic.

## Installation

To use this workflow from the source root, run the setup script once:

```bash
python3 hardware/google/aemu/setup_agents.py
```

## Specialized Agents

| Agent | Skill Name | Purpose |
| :--- | :--- | :--- |
| **Scribe** | `agent_intervention_scribe` | Passive chronicler; records human interventions. |
| **Evolver** | `agent_autonomy_evolver` | Mutation engine; hardens skills to increase autonomy. |
| **Git Expert** | `git_expert` | SCM Specialist; handles complex rebases and hygiene. |
| **Planner** | `planner` | Architect; handles requirements and `DESIGN.md`. |
| **Critic** | `critic` | Security & Efficiency reviewer; finds deep bugs. |
| **Reviewer** | `reviewer` | Quality gatekeeper; audits diffs for noise. |
| **Style Enforcer** | `style_enforcer` | Formatting & Style specialist. |
| **Documenter** | `documenter` | Knowledge specialist; ensures docs are up-to-date. |
| **Debugger** | `debugger` | Investigation specialist; uses GDB and logging. |
| **License Agent** | `license` | Compliance specialist; handles license preambles. |
| **Committer** | `committer` | Release specialist; crafts commit messages. |

## The Development Lifecycle

1.  **Research & Planning:** **MANDATORY:** Activate the **Scribe** (to log friction) and
    **Planner** (to design the solution) immediately.
1.1 **Engagement Mode Selection:** The agent assesses task complexity and **recommends** a mode, but the human makes the final selection:
    *   **Autonomous Mode:** Recommended for trivial tasks (1:1 mapping) or rapid Proof-of-Concepts where the human wants a "hands-off" exploration. The agent works independently until a Gerrit CL is ready.
    *   **Collaborative Mode:** Recommended for complex tasks involving architectural trade-offs or new patterns. The agent acts as a peer, pausing at major design forks to invite debate and ensure **Maximum Alignment** before execution.

    **Dynamic Mode Switching:** The engagement mode is not permanent. A human may start in **Autonomous Mode** to quickly see a rough implementation, then pivot to **Collaborative Mode** to refine the solution. Conversely, a human may start in **Collaborative Mode** to set the strategy and then switch to **Autonomous Mode** for the bulk of the implementation.
1.2 **Atomic Decomposition:** If complex, `planner` outlines a sequence of focused commits.
2.  **Design Review:** Activate `critic` to scrutinize the proposed design.
3.  **Implementation (TDD):** Implement code and tests.
    *   **Continuous Learning:** The **Scribe** records every human intervention.
4.  **Verification:** Run tests.
5.  **The Pivot & Debug:** If failed, activate `debugger` to use GDB/logging.
6.  **Refinement:** Activate `critic` and `style_enforcer` to polish the implementation.
7.  **Documentation:** Activate `documenter` to update relevant documentation.
8.  **Internal Review:** Activate `reviewer` to audit the diff and metadata.
9.  **Submission:** Activate `committer` to prepare the Gerrit patchset.
9.1 **Human Approval Gate:** The human reviews the commit.
    *   **If Approved:** **MANDATORY:** The AI must immediately pivot to Step 10.
    *   **If Rejected:** The **Scribe** logs the reason; the team diagnoses the failure.
10. **Autonomy Audit (Post-Mortem):** The **Evolver** MUST analyze the intervention log to
    harden skills and workflows. The task is not "Done" until the system has evolved.

## The Organizational Hierarchy

To use this system effectively, understand the chain of command:

| Level | Role | Responsibilities |
| :--- | :--- | :--- |
| **Executive** | CEO / Founder | Sets vision, defines project culture. |
| **Management** | Director | **Zero-Repeat Mandate:** Orchestrates specialists and logs friction. |
| **Specialist** | Staff Eng. | Performs execution: Planning, Security Audits, Coding, etc. |

## The Four Pillars of Reviewability

Every change produced by this team must adhere to these standards:

1.  **Semantic Atomicity:** One commit, one logical change. Never bundle unrelated features.
2.  **No-Surprise Diffs:** Every line in a diff must be a direct servant of the commit's subject.
3.  **Skeleton-First Progression:** Large features delivered in a layered stack.
4.  **Proactive Justification:** Explain *why* specific choices were made.

## Continuous Evolution: The Duo

The **Scribe** and **Evolver** are the heart of the system's growth, focused on eliminating **Instructional Debt** while preserving **Strategic Alignment**.

1.  **Job 1 (The Sensor):** The **Scribe** records every human interaction. It distinguishes between:
    *   **Instructional Debt:** Corrections to process, style, or known rules. These are targets for automation.
    *   **Collaborative Guidance:** Strategic intent, architectural trade-offs, or new context. These are targets for knowledge capture.
2.  **Job 2 (The Actuator):** Once approved, the **Evolver** analyzes the log to harden skills (eliminating debt) and update documentation/references (capturing guidance).
