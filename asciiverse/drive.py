"""drive.py SESSION: plays miscom in a 100x30 tmux pane for the catalog
preview (asciiverse/record.sh): starts a game from the title and
defends the cities.

The 80x24 screen is centred at 100x30 (its corner at row 3, column 10).
The play field is its rows 0 to 19, each row two half-rows ("`" upper,
"." lower, ":" both); incoming missiles are magenta, the player's
green, so the pane is read with its colours (tmux capture-pane -e). The
driver finds the warheads (the lowest magenta pixels of their trails),
takes the one nearest the cities, aims a little below it and fires. The
sight moves 6 columns or 5 half-rows a key and stops at the field's
edges, as the game does, so the driver follows where it is and aims at
the nearest place it can reach. Stops after SECONDS (default 65)."""
import os, re, subprocess, sys, time

S = sys.argv[1]
secs = float(os.environ.get("SECONDS_LIMIT", "65"))
TMUX = ["tmux", "-L", os.environ.get("TMUX_SOCKET", "default")]
OY, OX = 3, 10
MAX_X, MAX_Y = 71, 40
SGR = re.compile(r"\x1b\[([0-9;]*)m")
LEAD = int(os.environ.get("LEAD", "3"))      # half-rows to aim ahead


def key(*k):
    subprocess.run(TMUX + ["send-keys", "-t", S, *k])


def magenta_pixels():
    """{(x, y)} of the incoming missiles' pixels, in the game's units."""
    out = subprocess.run(TMUX + ["capture-pane", "-p", "-e", "-t", S],
                         capture_output=True, text=True).stdout.split("\n")
    pix = set()
    for row in range(20):
        if OY + row >= len(out):
            break
        line, fg, col, i = out[OY + row], None, 0, 0
        while i < len(line):
            m = SGR.match(line, i)
            if m:
                for code in (m.group(1) or "0").split(";"):
                    if code in ("0", "39", ""):
                        fg = None
                    elif code.isdigit() and 30 <= int(code) <= 37:
                        fg = int(code)
                i = m.end()
                continue
            ch = line[i]
            x = col - OX
            if fg == 35 and 0 <= x < MAX_X:
                if ch in "`:'":
                    pix.add((x, 2 * row))
                if ch in ".:,":
                    pix.add((x, 2 * row + 1))
            col += 1
            i += 1
    return pix


def heads(pix):
    """Warheads: pixels with no pixel of the trail below them."""
    return [(x, y) for x, y in pix
            if not any((x + dx, y + dy) in pix for dx in (-1, 0, 1) for dy in (1, 2))]


class Sight:
    """The sight, moved as miscom moves it."""
    def __init__(self):
        self.x, self.y = 35, 20

    def step(self, k):
        if k == "h": self.x = max(0, self.x - 6)
        if k == "l": self.x = min(MAX_X - 1, self.x + 6)
        if k == "k": self.y = max(0, self.y - 5)
        if k == "j": self.y = min(MAX_Y - 2, self.y + 5)

    def go(self, tx, ty):
        """Walk to the reachable place nearest (tx, ty)."""
        for _ in range(30):
            best, bk = (abs(self.x - tx) + abs(self.y - ty), None)
            for k in "hljk":
                s = Sight(); s.x, s.y = self.x, self.y; s.step(k)
                d = abs(s.x - tx) + abs(s.y - ty)
                if d < best:
                    best, bk = d, k
            if bk is None:
                return
            self.step(bk)
            key(bk)
            time.sleep(0.04)


time.sleep(1.5)
key("Enter")                    # the title: play
time.sleep(1.5)
sight = Sight()
t0 = time.time()
recent = []                     # (x, y, time) of recent shots
while time.time() - t0 < secs:
    pix = magenta_pixels()
    hs = heads(pix)
    now = time.time()
    recent = [r for r in recent if now - r[2] < 1.6]
    hs = [h for h in hs if not any(abs(h[0] - r[0]) < 6 and abs(h[1] - r[1]) < 7 for r in recent)]
    if hs:
        hx, hy = max(hs, key=lambda h: h[1])
        # its direction, from the trail a few half-rows back
        back = [p for p in pix if hy - 8 <= p[1] <= hy - 4 and abs(p[0] - hx) <= 8]
        if back:
            bx = sum(p[0] for p in back) / len(back)
            by = sum(p[1] for p in back) / len(back)
            dy = max(hy - by, 1)
            slope = (hx - bx) / dy
        else:
            slope = 0
        lead = LEAD
        tx, ty = round(hx + slope * lead), min(hy + lead, MAX_Y - 6)
        sight.go(tx, ty)
        key("Space")
        recent.append((tx, ty, now))
        time.sleep(0.15)
    else:
        time.sleep(0.12)
