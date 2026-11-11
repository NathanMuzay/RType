# Handover and team changes

Teams change: someone joins late, someone leaves, someone is absent for two weeks before a deadline. This page
lists what to do so that no work and no knowledge is lost.

## Someone joins the team

| Step | Who |
|---|---|
| Give repository write access and channel access | Any member |
| Point to [Onboarding](onboarding.md) | Any member |
| Pair for one hour on a build and a first change | The member closest to their area |
| Assign a first small issue | The team |
| Add their name to the README authors | The newcomer, in their first pull request |

## Someone leaves, or will be absent

Before leaving, the member:

1. **Pushes everything.** Every local branch, even unfinished, is pushed. Unfinished work is marked
   `WIP:` in its pull request title.
2. **Writes a handover note** in each open pull request or issue:

    ```text
    State: what works, what does not
    Next steps: the next 2-3 things to do
    Pitfalls: what took time, what to avoid
    How to test: commands and expected result
    ```

3. **Transfers ownership** of their areas in [Onboarding, who owns what](onboarding.md#who-owns-what).
4. **Updates the documentation** of everything they built. Code nobody else understands is a risk for the
   whole team.

The team then reassigns the open issues and keeps or closes the unfinished branches.

## Someone is unreachable

If a member stops answering with work in progress:

1. Look for their pushed branches: `git branch -r`, `git log origin/<branch>`.
2. Read the open pull requests and issues assigned to them.
3. Reassign the issues after a team decision. Do not delete their branches: archive them by keeping them.
4. Inform the pedagogical team if the workload of the project changes.

## Knowledge that must never live in one head

| Topic | Where it is written |
|---|---|
| How to build and run | [Getting started](../getting-started/installation.md) |
| Architecture choices | [Design decisions](../architecture/decisions.md) |
| Protocol | [RFC](../reference/protocol.md) |
| How to add content | [Guides](../guides/content/create-a-game.md) |
| Known problems | [Troubleshooting](../getting-started/troubleshooting.md), issues |
