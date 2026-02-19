# AEMU Agentic Development Workflow

This repository uses a multi-agent architecture to ensure high-quality engineering and
continuous improvement.

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
1.1 **Atomic Decomposition:** If complex, `planner` outlines a sequence of focused commits.
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

The **Scribe** and **Evolver** are the heart of the system's growth.

1.  **Job 1 (The Sensor):** The **Scribe** notes every interaction where the human had to correct,
    clarify, or provide context. These are the barriers to full autonomy.
2.  **Job 2 (The Actuator):** Once approved, the **Evolver** analyze these notes to eliminate future
    human interaction by hardening skills and workflow rules.
