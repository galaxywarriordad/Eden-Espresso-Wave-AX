# 🛡️ Git & Repository Safety Rules (`GIT_RULES.md`)

**Scope:** Local Workstation → Primary Forgejo Server → Read-Only GitHub Mirror  
**Core Principle:** Your local working directory is the **single source of truth**. Remote servers are strictly downstream storage and publishing targets.

---

## 1. The Five Golden Rules

1. **Never Edit Files via Web UIs:** Never edit `.md` files, configs, or code directly in the Forgejo or GitHub browser editors. Every single edit happens locally on your machine.
2. **One-Way Flow Only (`Local → Forgejo → GitHub`):** Never pull, merge, or sync *from* GitHub back into Forgejo or your local machine. GitHub is a **read-only downstream mirror**.
3. **No Automated Git Wrapper Scripts:** Never run custom PowerShell or Bash scripts that execute `git reset`, `git clean`, `git checkout`, or `git push`. Type explicit `git` commands manually so you see every action.
4. **Backup Before Every Git Operation:** Before running `git commit`, `git push`, `git rebase`, or `git merge`, take a manual read-only copy of your working folder to a separate physical drive.
5. **Never Force-Push Protected Branches:** Keep `--force` / `-f` disabled on `master` / `main` across both Forgejo and GitHub.

---

## 2. Manual Local Archiving Workflow

Perform this simple, read-only backup routine before testing new builds or touching `git`:

1. **Snapshot Your Working Tree (Read-Only):**
   * Copy your project folder (or right-click → **7-Zip → Add to archive...**) to a timestamped folder on a separate physical drive (e.g., `D:\Backups\Eden-AX_2026-11-18_Stage3_Booting.7z`).
   * Keep at least the **last 3 known-good snapshots** before deleting older ones.
2. **Verify Zero-Byte Files Before Committing:**
   * Open Windows File Explorer in `src\`, sort by **Size** ascending, and confirm no `.cpp`, `.h`, or `.hpp` files are `0 KB` before staging anything.

---

## 3. Safe Daily Git Commands (Manual Checklist)

Run these commands explicitly in your terminal—never wrapped in automation:

### Step 1: Inspect What Changed
```powershell
git status
git diff --stat
```
*Confirm that only the exact files you intended to modify appear in the list, and no files show unexpected deletions.*

### Step 2: Stage Deliberately
```powershell
git add path\(\to\specific_file.\)cpp
```
*Avoid blind `git add .` after large batch runs or build sweeps until `git status` confirms no build logs, `.profraw` dumps, or temporary artefacts are untracked.*

### Step 3: Commit Locally
```powershell
git commit -m "feat(topology): isolate windows.h behind ax_platform_shim"
```

### Step 4: Push One-Way to Primary Remote (Forgejo)
```powershell
git push origin master
```
*Let Forgejo's built-in **push-only mirror** update GitHub automatically. Never push directly to GitHub from your local machine unless Forgejo is offline.*

---

## 4. Remote Configuration Checklist (For Your Clean Repo)

When you stand up your fresh, clean repository after Stage 3 testing succeeds:

* **On GitHub (Read-Only Mirror):**
  * Go to **Settings → Rules / Branches** and enable branch protection on `master` (block force-pushes and deletions).
  * Never enable GitHub Actions workflows that commit back to the repository.
* **On Forgejo (Primary Server):**
  * Configure **Repository Settings → Mirror Settings** strictly as **Push Mirror** (`Forgejo → GitHub`).
  * Ensure **Pull Mirror** is completely disabled.
* **On Local Machine (`git remote -v`):**
  * Keep only `origin` pointing to your Forgejo repository so you never accidentally push to or pull from the wrong remote.

---

## 5. Emergency Recovery Rule

If a remote ever looks out of sync or corrupted:
1. **Stop immediately**—do **not** run `git pull`, `git reset`, or `git clean` locally.
2. Copy your current local folder to a safe backup drive first (`D:\Backups\Emergency_Copy\`).
3. Verify your local files are intact, then push your verified local state outward—or recreate the remote mirror from your intact local copy.
