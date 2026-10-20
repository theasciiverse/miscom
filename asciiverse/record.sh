#!/bin/sh
# Records asciiverse/miscom.cast, the catalog preview, at 100x30:
#
#   asciiverse/record.sh FOLDER
#
# FOLDER holds miscom as packed (the program and sounds/). asciinema rec
# runs it (by name, from a throwaway copy on PATH, with a throwaway home)
# in a detached 100x30 tmux session while asciiverse/drive.py plays. A
# stand-in aplay that plays nothing comes first on PATH, so recording
# makes no sound here. Then the game is stopped with SIGKILL, so the cast
# ends on the game, its last frame held for two seconds.
set -eu
cd "$(dirname "$0")/.."
src=$(cd "$1" && pwd)
out=$PWD/asciiverse/miscom.cast
S=miscom-preview-$$
home=$(mktemp -d)
trap 'tmux -L "$S" kill-server 2>/dev/null; rm -rf "$home"' EXIT
mkdir "$home/bin"
cp -r "$src"/. "$home/bin/"
printf '#!/bin/sh\nexec cat >/dev/null\n' > "$home/bin/aplay"
chmod +x "$home/bin/aplay"
tmux -L "$S" new-session -d -s "$S" -x 100 -y 30 -c "$home" \
  "env -u TMUX HOME=$home PATH=$home/bin:$PATH TERM=xterm-256color LANG=C.UTF-8 USER=player LOGNAME=player \
   asciinema rec --overwrite -q --window-size 100x30 --idle-time-limit 2 --title miscom \
     --capture-env TERM,LANG -c 'exec miscom' $out"
TMUX_SOCKET=$S python3 asciiverse/drive.py "$S"
pids=$(tmux -L "$S" display-message -p -t "$S" '#{pane_pid}')
for _ in 1 2 3; do
  pids="$pids $(for p in $pids; do pgrep -P "$p" || true; done)"
done
for p in $(echo $pids | tr ' ' '\n' | sort -u); do
  [ "$(ps -o comm= -p "$p" 2>/dev/null)" = miscom ] && kill -KILL "$p"
done
for _ in $(seq 1 20); do
  tmux -L "$S" has-session -t "$S" 2>/dev/null || break
  sleep 0.5
done
python3 - "$out" <<'PY'
import json, sys
lines = open(sys.argv[1]).read().splitlines()
keep = [lines[0]]
for line in lines[1:]:
    ev = json.loads(line)
    if ev[1] == "x" or (ev[1] == "o" and "\x1b[?1049l" in ev[2]):
        break
    keep.append(line)
keep.append(json.dumps([2.0, "o", ""]))
open(sys.argv[1], "w").write("\n".join(keep) + "\n")
PY
echo "recorded $out"
