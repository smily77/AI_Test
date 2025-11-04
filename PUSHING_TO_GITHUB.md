# Pushing This Repository to GitHub

> **Wichtig:** In dieser Umgebung kann ich nicht selbst eine Verbindung zu deinem GitHub-Account herstellen oder etwas hochladen, weil keine Zugangsdaten vorhanden sind. Die folgenden Schritte musst du daher lokal auf deinem Rechner ausführen.

This project includes a helper script, `push_to_github.sh`, that automates the steps you asked for: configuring the `origin` remote and pushing the current branch.

## Usage

```bash
./push_to_github.sh https://github.com/<dein-account>/<repo-name>.git [branch]
```

- Replace `https://github.com/<dein-account>/<repo-name>.git` with the HTTPS URL of your GitHub repository.
- The script defaults to pushing the currently checked-out branch. You can optionally pass a branch name as the second argument.

The script will:
1. Ensure it is being run inside a Git repository.
2. Configure (or update) the `origin` remote to the provided URL.
3. Push the branch to GitHub with `git push -u origin <branch>` so that future `git push`/`git pull` commands work without extra arguments.

Run the script from the project root and enter your GitHub credentials or token when prompted by Git.

## Repository lokal ausführen

Falls das Projekt bislang nur hier im Arbeitsbereich existiert, musst du es einmalig auf deinen Rechner holen, bevor du den Push
machen kannst. Das geht am bequemsten per `git clone`:

```bash
git clone <dein-remote-arbeitsbereich-url> AI_Test
cd AI_Test
```

Alternativ kannst du das Projekt als ZIP herunterladen und entpacken oder die Dateien per `scp` kopieren. Entscheidend ist nur,
dass der Push-Befehl von einem System ausgeführt wird, das sich mit deinem GitHub-Konto authentifizieren kann – also dein eigener
Rechner.

### "Wo kann ich das ZIP herunterladen?"

- Direkt aus diesem Arbeitsbereich kannst du **kein** ZIP herunterladen, weil die Umgebung nicht von außen erreichbar ist.
- Sobald du die Dateien in deine eigene Umgebung kopiert hast (z. B. durch `git clone` aus einer Quelle, auf die du Zugriff hast), kannst du dort selbst ein ZIP erstellen, zum Beispiel mit `zip -r AI_Test.zip AI_Test/`.
- Wenn du das Repository nach GitHub gepusht hast, stellt GitHub automatisch einen "Download ZIP"-Knopf im Webinterface bereit.

### Warum du **nicht** meinen Arbeitsbereich klonen kannst

- Der hier bereitgestellte Arbeitsbereich ist isoliert und von außen nicht erreichbar; es gibt also keine URL, die du klonen könntest.
- Selbst wenn es eine Adresse gäbe, dürften hier keine persönlichen Zugangsdaten hinterlegt werden. Ohne eigene Credentials würdest du also keine Verbindung aufbauen können.

Kurz gesagt: Du musst die Dateien in deine eigene Umgebung übernehmen (per `git clone` aus einem von dir kontrollierten Speicherort oder per Download) und von dort zu GitHub pushen.
