#!/usr/bin/env bash
set -euo pipefail

# Publicacao intencional: executada localmente pelo usuario, nunca por CI.
repo_name="KartCometaRegenerativo"
if ! command -v git >/dev/null; then
  echo 'Instale git antes de publicar.' >&2
  exit 1
fi
if ! command -v gh >/dev/null; then
  echo 'GitHub CLI (gh) nao instalado. Instale https://cli.github.com/ e rode gh auth login.' >&2
  exit 1
fi
if ! gh auth status >/dev/null 2>&1; then
  echo 'Autentique primeiro: gh auth login' >&2
  exit 1
fi
if [[ ! -d .git ]]; then
  git init -b main
fi
if [[ -z "$(git config user.name || true)" || -z "$(git config user.email || true)" ]]; then
  echo 'Configure git user.name e user.email antes de publicar.' >&2
  exit 1
fi
git add .
if ! git diff --cached --quiet; then
  git commit -m 'feat: implementar simulador Cometa Phoenix com regeneracao e interface'
fi
if ! git remote get-url origin >/dev/null 2>&1; then
  echo 'Criando repositorio PRIVADO novo no GitHub (sem substituir outros repositorios)...'
  gh repo create "$repo_name" --private --source=. --remote=origin --push
else
  echo 'Remote origin ja existe. Conferindo antes de publicar:'
  git remote -v
  echo 'Execute manualmente: git push -u origin HEAD'
  echo 'Nenhum push automatico feito em repositorio existente.'
fi
