// Pac-Man Arcade Universal Engine
// Developer: Md. Abu Rise Zunaed
// Faithful implementation of C++17 / SDL2 arcade edition

const COLS = 28;
const ROWS = 31;
const TILE_SIZE = 24;
const HEADER_HEIGHT = 60;
const FOOTER_HEIGHT = 46;
const WINDOW_WIDTH = COLS * TILE_SIZE; // 672
const WINDOW_HEIGHT = HEADER_HEIGHT + (ROWS * TILE_SIZE) + FOOTER_HEIGHT; // 850

const RAW_MAZE = [
  "1111111111111111111111111111",
  "1222222222222112222222222221",
  "1211112111112112111112111121",
  "1311112111112112111112111131",
  "1211112111112112111112111121",
  "1222222222222222222222222221",
  "1211112112111111112112111121",
  "1211112112111111112112111121",
  "1222222112222112222112222221",
  "1111112111110110111112111111",
  "0000012111110110111112100000",
  "0000012110000000000112100000",
  "0000012110111441110112100000",
  "1111112110155555510112111111",
  "6000002000155555510002000006",
  "1111112110155555510112111111",
  "0000012110111111110112100000",
  "0000012110000000000112100000",
  "0000012110111111110112100000",
  "1111112110111111110112111111",
  "1222222222222112222222222221",
  "1211112111112112111112111121",
  "1211112111112112111112111121",
  "1322112222222002222222112231",
  "1112112112111111112112112111",
  "1112112112111111112112112111",
  "1222222112222112222112222221",
  "1211111111112112111111111121",
  "1211111111112112111111111121",
  "1222222222222222222222222221",
  "1111111111111111111111111111"
];

const STAGE_FRUITS = [
  { name: "Cherry", points: 100, color: "#ef4444" },
  { name: "Strawberry", points: 300, color: "#f43f5e" },
  { name: "Peach", points: 500, color: "#fb923c" },
  { name: "Apple", points: 700, color: "#22c55e" },
  { name: "Grapes", points: 1000, color: "#a855f7" },
  { name: "Galaxian", points: 2000, color: "#38bdf8" },
  { name: "Bell", points: 3000, color: "#eab308" },
  { name: "Key", points: 5000, color: "#06b6d4" }
];

const INTRO_MELODY = [
  { freq: 493.88, dur: 0.14 }, { freq: 987.77, dur: 0.14 }, { freq: 739.99, dur: 0.14 }, { freq: 622.25, dur: 0.14 },
  { freq: 987.77, dur: 0.08 }, { freq: 739.99, dur: 0.18 }, { freq: 622.25, dur: 0.22 },
  { freq: 523.25, dur: 0.14 }, { freq: 1046.50, dur: 0.14 }, { freq: 783.99, dur: 0.14 }, { freq: 659.25, dur: 0.14 },
  { freq: 1046.50, dur: 0.08 }, { freq: 783.99, dur: 0.18 }, { freq: 659.25, dur: 0.22 },
  { freq: 493.88, dur: 0.14 }, { freq: 987.77, dur: 0.14 }, { freq: 739.99, dur: 0.14 }, { freq: 622.25, dur: 0.14 },
  { freq: 987.77, dur: 0.08 }, { freq: 739.99, dur: 0.18 }, { freq: 622.25, dur: 0.22 },
  { freq: 622.25, dur: 0.08 }, { freq: 659.25, dur: 0.08 }, { freq: 698.46, dur: 0.08 }, { freq: 698.46, dur: 0.08 },
  { freq: 739.99, dur: 0.08 }, { freq: 783.99, dur: 0.08 }, { freq: 830.61, dur: 0.08 }, { freq: 880.00, dur: 0.08 },
  { freq: 987.77, dur: 0.40 }
];

// Web Audio Polyphonic Synthesizer
class RetroAudioEngine {
  constructor() {
    this.ctx = null;
    this.muted = false;
    this.sirenOsc = null;
    this.sirenGain = null;
    this.sirenLfo = null;
    this.sirenMode = 0;
  }

  ensureContext() {
    if (!this.ctx) {
      const AudioCtx = window.AudioContext || window.webkitAudioContext;
      if (AudioCtx) this.ctx = new AudioCtx();
    }
    if (this.ctx && this.ctx.state === 'suspended') {
      this.ctx.resume();
    }
  }

  playIntro() {
    if (this.muted) return;
    this.ensureContext();
    if (!this.ctx) return;

    this.setSiren(0);
    let curTime = this.ctx.currentTime + 0.05;
    INTRO_MELODY.forEach(note => {
      const osc = this.ctx.createOscillator();
      const gain = this.ctx.createGain();
      osc.type = 'triangle';
      osc.frequency.setValueAtTime(note.freq, curTime);
      gain.gain.setValueAtTime(0.2, curTime);
      gain.gain.exponentialRampToValueAtTime(0.001, curTime + note.dur * 0.95);
      osc.connect(gain);
      gain.connect(this.ctx.destination);
      osc.start(curTime);
      osc.stop(curTime + note.dur);
      curTime += note.dur;
    });
  }

  setSiren(mode) {
    if (this.sirenMode === mode) return;
    this.sirenMode = mode;
    if (this.sirenOsc) {
      try {
        this.sirenOsc.stop();
        this.sirenLfo.stop();
      } catch (e) {}
      this.sirenOsc = null;
      this.sirenLfo = null;
    }
    if (mode === 0 || this.muted) return;
    this.ensureContext();
    if (!this.ctx) return;

    this.sirenOsc = this.ctx.createOscillator();
    this.sirenGain = this.ctx.createGain();
    this.sirenLfo = this.ctx.createOscillator();
    const lfoGain = this.ctx.createGain();

    let baseFreq = 250, sweep = 50, rate = 1.8, vol = 0.06;
    if (mode === 2) { baseFreq = 220; sweep = 70; rate = 4.5; vol = 0.07; }
    if (mode === 3) { baseFreq = 540; sweep = 180; rate = 9.0; vol = 0.09; }

    this.sirenOsc.type = 'sine';
    this.sirenOsc.frequency.setValueAtTime(baseFreq, this.ctx.currentTime);
    this.sirenLfo.type = 'sine';
    this.sirenLfo.frequency.setValueAtTime(rate, this.ctx.currentTime);
    lfoGain.gain.setValueAtTime(sweep, this.ctx.currentTime);

    this.sirenLfo.connect(lfoGain);
    lfoGain.connect(this.sirenOsc.frequency);

    this.sirenGain.gain.setValueAtTime(vol, this.ctx.currentTime);
    this.sirenOsc.connect(this.sirenGain);
    this.sirenGain.connect(this.ctx.destination);

    this.sirenOsc.start();
    this.sirenLfo.start();
  }

  playChomp(high) {
    if (this.muted) return;
    this.ensureContext();
    if (!this.ctx) return;

    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();
    const now = this.ctx.currentTime;
    osc.type = 'triangle';
    osc.frequency.setValueAtTime(high ? 490 : 370, now);
    osc.frequency.exponentialRampToValueAtTime(high ? 240 : 180, now + 0.08);

    gain.gain.setValueAtTime(0.2, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.08);

    osc.connect(gain);
    gain.connect(this.ctx.destination);
    osc.start(now);
    osc.stop(now + 0.09);
  }

  playEatGhost() {
    if (this.muted) return;
    this.ensureContext();
    if (!this.ctx) return;

    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();
    const now = this.ctx.currentTime;
    osc.type = 'square';
    osc.frequency.setValueAtTime(320, now);
    osc.frequency.exponentialRampToValueAtTime(960, now + 0.35);

    gain.gain.setValueAtTime(0.25, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.35);

    osc.connect(gain);
    gain.connect(this.ctx.destination);
    osc.start(now);
    osc.stop(now + 0.35);
  }

  playFruit() {
    if (this.muted) return;
    this.ensureContext();
    if (!this.ctx) return;

    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();
    const now = this.ctx.currentTime;
    osc.type = 'triangle';
    osc.frequency.setValueAtTime(587, now);
    osc.frequency.exponentialRampToValueAtTime(1175, now + 0.28);

    gain.gain.setValueAtTime(0.22, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.28);

    osc.connect(gain);
    gain.connect(this.ctx.destination);
    osc.start(now);
    osc.stop(now + 0.28);
  }

  playDeath() {
    if (this.muted) return;
    this.setSiren(0);
    this.ensureContext();
    if (!this.ctx) return;

    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();
    const now = this.ctx.currentTime;
    osc.type = 'triangle';
    osc.frequency.setValueAtTime(620, now);
    osc.frequency.exponentialRampToValueAtTime(90, now + 0.85);

    gain.gain.setValueAtTime(0.3, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.85);

    osc.connect(gain);
    gain.connect(this.ctx.destination);
    osc.start(now);
    osc.stop(now + 0.85);
  }

  toggleMute() {
    this.muted = !this.muted;
    if (this.muted) {
      if (this.sirenGain) this.sirenGain.gain.setValueAtTime(0, this.ctx.currentTime);
    } else {
      if (this.sirenMode > 0) {
        const m = this.sirenMode;
        this.sirenMode = 0;
        this.setSiren(m);
      }
    }
    return this.muted;
  }
}

// Game Implementation
const audio = new RetroAudioEngine();
const canvas = document.getElementById('gameCanvas');
const ctx = canvas.getContext('2d');

let maze = [];
let pacman = {
  x: 13.5 * TILE_SIZE,
  y: 23.0 * TILE_SIZE + TILE_SIZE / 2,
  dir: 'LEFT',
  nextDir: 'LEFT',
  speed: 2.4,
  mouthTan: 0.35,
  mouthClosing: false,
  deathProgress: 0.0
};

let ghosts = [
  { name: 'Blinky', color: '#ef4444', x: 13.5 * TILE_SIZE, y: 11.0 * TILE_SIZE + 12, startX: 13.5 * TILE_SIZE, startY: 11.0 * TILE_SIZE + 12, target: [27, 0], state: 'SCATTER', dir: 'LEFT', speed: 2.0 },
  { name: 'Pinky',  color: '#f472b6', x: 13.5 * TILE_SIZE, y: 14.0 * TILE_SIZE + 12, startX: 13.5 * TILE_SIZE, startY: 14.0 * TILE_SIZE + 12, target: [2, 0],   state: 'IN_HOUSE', dir: 'UP', speed: 2.0 },
  { name: 'Inky',   color: '#06b6d4', x: 11.5 * TILE_SIZE, y: 14.0 * TILE_SIZE + 12, startX: 11.5 * TILE_SIZE, startY: 14.0 * TILE_SIZE + 12, target: [27, 31], state: 'IN_HOUSE', dir: 'UP', speed: 2.0 },
  { name: 'Clyde',  color: '#f97316', x: 15.5 * TILE_SIZE, y: 14.0 * TILE_SIZE + 12, startX: 15.5 * TILE_SIZE, startY: 14.0 * TILE_SIZE + 12, target: [0, 31],  state: 'IN_HOUSE', dir: 'UP', speed: 2.0 }
];

let floatingScores = [];
let state = 'START_SCREEN';
let score = 0;
let highScore = parseInt(localStorage.getItem('pacman_highscore') || '0', 10);
let lives = 3;
let level = 1;
let dotsLeft = 0;
let totalDots = 0;
let frightenedTimer = 0;
let ghostsEatenMultiplier = 1;
let modeTimer = 0;
let globalGhostMode = 'SCATTER';

let fruitActive = false;
let fruitTimer = 0;
let fruitIndex = 0;
let chompHigh = false;
let freezeTimer = 0;
let clearFlashWhite = false;
let clearFlashCount = 0;

function initMaze() {
  maze = [];
  dotsLeft = 0;
  for (let r = 0; r < ROWS; r++) {
    const row = [];
    for (let c = 0; c < COLS; c++) {
      const val = parseInt(RAW_MAZE[r][c], 10);
      row.push(val);
      if (val === 2 || val === 3) dotsLeft++;
    }
    maze.push(row);
  }
  totalDots = dotsLeft;
}

function resetPositions() {
  pacman.x = 13.5 * TILE_SIZE;
  pacman.y = 23.0 * TILE_SIZE + TILE_SIZE / 2;
  pacman.dir = 'LEFT';
  pacman.nextDir = 'LEFT';
  pacman.mouthTan = 0.35;
  pacman.mouthClosing = false;
  pacman.deathProgress = 0.0;

  ghosts[0].x = ghosts[0].startX; ghosts[0].y = ghosts[0].startY; ghosts[0].dir = 'LEFT'; ghosts[0].state = 'SCATTER';
  ghosts[1].x = ghosts[1].startX; ghosts[1].y = ghosts[1].startY; ghosts[1].dir = 'UP';   ghosts[1].state = 'IN_HOUSE';
  ghosts[2].x = ghosts[2].startX; ghosts[2].y = ghosts[2].startY; ghosts[2].dir = 'UP';   ghosts[2].state = 'IN_HOUSE';
  ghosts[3].x = ghosts[3].startX; ghosts[3].y = ghosts[3].startY; ghosts[3].dir = 'UP';   ghosts[3].state = 'IN_HOUSE';

  frightenedTimer = 0;
  modeTimer = 0;
  globalGhostMode = 'SCATTER';
  fruitActive = false;
  clearFlashWhite = false;
  clearFlashCount = 0;
  audio.setSiren(0);
}

function startNewGame() {
  score = 0;
  lives = 3;
  level = 1;
  initMaze();
  resetPositions();
  state = 'READY';
  freezeTimer = 240;
  audio.playIntro();
}

function isTilePassable(tx, ty, d, isGhost = false, gst = 'CHASE') {
  let nx = tx, ny = ty;
  if (d === 'UP') ny--;
  else if (d === 'DOWN') ny++;
  else if (d === 'LEFT') nx--;
  else if (d === 'RIGHT') nx++;

  if (ny === 14 && (nx < 0 || nx >= COLS)) return true;
  if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS) return false;

  const cell = maze[ny][nx];
  if (cell === 1) return false;
  if (cell === 4) return isGhost && (gst === 'LEAVING_HOUSE' || gst === 'EATEN');
  if (cell === 5) return isGhost && (gst === 'IN_HOUSE' || gst === 'LEAVING_HOUSE' || gst === 'EATEN');
  return true;
}

function updatePacman() {
  const tileX = Math.floor(pacman.x / TILE_SIZE);
  const tileY = Math.floor(pacman.y / TILE_SIZE);
  const centerX = tileX * TILE_SIZE + TILE_SIZE / 2;
  const centerY = tileY * TILE_SIZE + TILE_SIZE / 2;

  // Turnaround
  const opposites = { UP: 'DOWN', DOWN: 'UP', LEFT: 'RIGHT', RIGHT: 'LEFT' };
  if (opposites[pacman.dir] === pacman.nextDir) {
    pacman.dir = pacman.nextDir;
  }

  const distToCenter = Math.hypot(pacman.x - centerX, pacman.y - centerY);
  if (pacman.nextDir !== pacman.dir && distToCenter < 6.0) {
    if (isTilePassable(tileX, tileY, pacman.nextDir)) {
      pacman.x = centerX;
      pacman.y = centerY;
      pacman.dir = pacman.nextDir;
    }
  }

  let blocked = false;
  if (distToCenter < pacman.speed) {
    if (!isTilePassable(tileX, tileY, pacman.dir)) {
      pacman.x = centerX;
      pacman.y = centerY;
      blocked = true;
    }
  }

  if (!blocked) {
    if (pacman.dir === 'LEFT') { pacman.y = centerY; pacman.x -= pacman.speed; }
    else if (pacman.dir === 'RIGHT') { pacman.y = centerY; pacman.x += pacman.speed; }
    else if (pacman.dir === 'UP') { pacman.x = centerX; pacman.y -= pacman.speed; }
    else if (pacman.dir === 'DOWN') { pacman.x = centerX; pacman.y += pacman.speed; }

    if (pacman.mouthClosing) {
      pacman.mouthTan -= 0.035;
      if (pacman.mouthTan <= 0.05) pacman.mouthClosing = false;
    } else {
      pacman.mouthTan += 0.035;
      if (pacman.mouthTan >= 0.65) pacman.mouthClosing = true;
    }
  }

  // Tunnel wrap
  if (tileY === 14) {
    if (pacman.x < -TILE_SIZE / 2) pacman.x = WINDOW_WIDTH + TILE_SIZE / 2;
    else if (pacman.x > WINDOW_WIDTH + TILE_SIZE / 2) pacman.x = -TILE_SIZE / 2;
  }

  // Eating pellets
  const curTileX = Math.floor(pacman.x / TILE_SIZE);
  const curTileY = Math.floor(pacman.y / TILE_SIZE);
  if (curTileX >= 0 && curTileX < COLS && curTileY >= 0 && curTileY < ROWS) {
    const cell = maze[curTileY][curTileX];
    if (cell === 2) {
      maze[curTileY][curTileX] = 0;
      dotsLeft--;
      score += 10;
      chompHigh = !chompHigh;
      audio.playChomp(chompHigh);
      checkFruit();
      checkWin();
    } else if (cell === 3) {
      maze[curTileY][curTileX] = 0;
      dotsLeft--;
      score += 50;
      frightenedTimer = 480;
      ghostsEatenMultiplier = 1;
      audio.playChomp(true);
      audio.setSiren(2);
      ghosts.forEach(g => {
        if (g.state !== 'EATEN' && g.state !== 'IN_HOUSE' && g.state !== 'LEAVING_HOUSE') {
          g.state = 'FRIGHTENED';
        }
      });
      checkFruit();
      checkWin();
    }
  }

  // Eat fruit
  if (fruitActive) {
    const fx = 13.5 * TILE_SIZE;
    const fy = 17.5 * TILE_SIZE;
    if (Math.hypot(pacman.x - fx, pacman.y - fy) < TILE_SIZE * 0.8) {
      fruitActive = false;
      const pts = STAGE_FRUITS[fruitIndex].points;
      score += pts;
      floatingScores.push({ x: fx, y: fy, text: '+' + pts, alpha: 1.0 });
      audio.playFruit();
    }
  }

  if (score > highScore) {
    highScore = score;
    localStorage.setItem('pacman_highscore', highScore.toString());
  }
}

function checkFruit() {
  const eaten = totalDots - dotsLeft;
  if ((eaten === 70 || eaten === 170) && !fruitActive) {
    fruitActive = true;
    fruitTimer = 600;
    fruitIndex = Math.min(level - 1, STAGE_FRUITS.length - 1);
  }
}

function checkWin() {
  if (dotsLeft <= 0) {
    state = 'LEVEL_CLEAR';
    clearFlashCount = 0;
    clearFlashWhite = false;
    freezeTimer = 20;
    audio.setSiren(0);
  }
}

function getGhostTarget(g) {
  const pTx = Math.floor(pacman.x / TILE_SIZE);
  const pTy = Math.floor(pacman.y / TILE_SIZE);
  if (g.state === 'EATEN') return [13, 11];
  if (g.state === 'SCATTER') return g.target;
  if (g.state === 'CHASE') {
    if (g.name === 'Blinky') return [pTx, pTy];
    if (g.name === 'Pinky') {
      const dx = (pacman.dir === 'RIGHT' ? 4 : (pacman.dir === 'LEFT' ? -4 : 0));
      const dy = (pacman.dir === 'DOWN' ? 4 : (pacman.dir === 'UP' ? -4 : 0));
      return [pTx + dx, pTy + dy];
    }
    if (g.name === 'Inky') {
      const bTx = Math.floor(ghosts[0].x / TILE_SIZE);
      const bTy = Math.floor(ghosts[0].y / TILE_SIZE);
      const ox = pTx + (pacman.dir === 'RIGHT' ? 2 : (pacman.dir === 'LEFT' ? -2 : 0));
      const oy = pTy + (pacman.dir === 'DOWN' ? 2 : (pacman.dir === 'UP' ? -2 : 0));
      return [ox + (ox - bTx), oy + (oy - bTy)];
    }
    if (g.name === 'Clyde') {
      const gTx = Math.floor(g.x / TILE_SIZE);
      const gTy = Math.floor(g.y / TILE_SIZE);
      if (Math.hypot(gTx - pTx, gTy - pTy) > 8) return [pTx, pTy];
      return g.target;
    }
  }
  return [pTx, pTy];
}

function updateGhosts() {
  if (frightenedTimer > 0) {
    frightenedTimer--;
    if (frightenedTimer === 0) {
      audio.setSiren(1);
      ghosts.forEach(g => { if (g.state === 'FRIGHTENED') g.state = globalGhostMode; });
    }
  } else {
    modeTimer++;
    if (modeTimer < 420) globalGhostMode = 'SCATTER';
    else if (modeTimer < 1620) globalGhostMode = 'CHASE';
    else if (modeTimer < 2040) globalGhostMode = 'SCATTER';
    else globalGhostMode = 'CHASE';

    ghosts.forEach(g => {
      if (g.state === 'SCATTER' || g.state === 'CHASE') g.state = globalGhostMode;
    });
  }

  const eaten = totalDots - dotsLeft;
  if (ghosts[1].state === 'IN_HOUSE') ghosts[1].state = 'LEAVING_HOUSE';
  if (eaten >= 30 && ghosts[2].state === 'IN_HOUSE') ghosts[2].state = 'LEAVING_HOUSE';
  if (eaten >= 60 && ghosts[3].state === 'IN_HOUSE') ghosts[3].state = 'LEAVING_HOUSE';

  const hasEyes = ghosts.some(g => g.state === 'EATEN');
  if (state === 'PLAYING') {
    if (hasEyes) audio.setSiren(3);
    else if (frightenedTimer > 0) audio.setSiren(2);
    else audio.setSiren(1);
  }

  ghosts.forEach(g => {
    let spd = g.speed;
    const tileX = Math.floor(g.x / TILE_SIZE);
    const tileY = Math.floor(g.y / TILE_SIZE);

    if (tileY === 14 && (tileX < 6 || tileX > 21)) spd = 1.1;
    if (g.state === 'FRIGHTENED') spd = 1.2;
    if (g.state === 'EATEN') spd = 4.2;

    if (g.state === 'IN_HOUSE') {
      g.y += (g.bounceDir || -1) * 0.6;
      if (g.y <= 13.8 * TILE_SIZE) g.bounceDir = 1;
      if (g.y >= 14.5 * TILE_SIZE) g.bounceDir = -1;
      return;
    }

    if (g.state === 'LEAVING_HOUSE') {
      const targetX = 13.5 * TILE_SIZE;
      const targetY = 11.0 * TILE_SIZE + 12;
      if (Math.abs(g.x - targetX) > 1.0) {
        g.x += (targetX > g.x ? 1.2 : -1.2);
      } else {
        g.x = targetX;
        g.y -= 1.4;
        if (g.y <= targetY) {
          g.y = targetY;
          g.state = globalGhostMode;
          g.dir = 'LEFT';
        }
      }
      return;
    }

    if (g.state === 'EATEN') {
      const doorX = 13.5 * TILE_SIZE;
      const doorY = 11.0 * TILE_SIZE + 12;
      if (Math.hypot(g.x - doorX, g.y - doorY) < 6.0) {
        g.x = doorX;
        g.y = 14.0 * TILE_SIZE;
        g.state = 'LEAVING_HOUSE';
        return;
      }
    }

    const centerX = tileX * TILE_SIZE + TILE_SIZE / 2;
    const centerY = tileY * TILE_SIZE + TILE_SIZE / 2;
    const distToCenter = Math.hypot(g.x - centerX, g.y - centerY);

    if (distToCenter < spd && (tileX !== g.lastTx || tileY !== g.lastTy)) {
      g.x = centerX;
      g.y = centerY;
      g.lastTx = tileX;
      g.lastTy = tileY;

      const opposites = { UP: 'DOWN', DOWN: 'UP', LEFT: 'RIGHT', RIGHT: 'LEFT' };
      const rev = opposites[g.dir];
      const order = ['UP', 'LEFT', 'DOWN', 'RIGHT'];
      const choices = order.filter(d => d !== rev && isTilePassable(tileX, tileY, d, true, g.state));
      if (choices.length === 0) choices.push(rev);

      if (g.state === 'FRIGHTENED') {
        g.dir = choices[Math.floor(Math.random() * choices.length)];
      } else {
        const target = getGhostTarget(g);
        let bestDir = choices[0];
        let bestDist = 1e9;
        choices.forEach(d => {
          let nx = tileX + (d === 'RIGHT' ? 1 : (d === 'LEFT' ? -1 : 0));
          let ny = tileY + (d === 'DOWN' ? 1 : (d === 'UP' ? -1 : 0));
          const dist = Math.hypot(nx - target[0], ny - target[1]);
          if (dist < bestDist) {
            bestDist = dist;
            bestDir = d;
          }
        });
        g.dir = bestDir;
      }
    }

    if (g.dir === 'LEFT') { g.y = centerY; g.x -= spd; }
    else if (g.dir === 'RIGHT') { g.y = centerY; g.x += spd; }
    else if (g.dir === 'UP') { g.x = centerX; g.y -= spd; }
    else if (g.dir === 'DOWN') { g.x = centerX; g.y += spd; }

    if (tileY === 14) {
      if (g.x < -TILE_SIZE / 2) g.x = WINDOW_WIDTH + TILE_SIZE / 2;
      else if (g.x > WINDOW_WIDTH + TILE_SIZE / 2) g.x = -TILE_SIZE / 2;
    }
  });
}

function checkCollisions() {
  for (const g of ghosts) {
    const dist = Math.hypot(pacman.x - g.x, pacman.y - g.y);
    if (dist < TILE_SIZE * 0.75) {
      if (g.state === 'FRIGHTENED') {
        g.state = 'EATEN';
        const pts = Math.min(1600, 200 * ghostsEatenMultiplier);
        ghostsEatenMultiplier *= 2;
        score += pts;
        floatingScores.push({ x: g.x, y: g.y, text: '+' + pts, alpha: 1.0 });
        audio.playEatGhost();
        state = 'GHOST_PAUSE';
        freezeTimer = 25;
        break;
      } else if (g.state === 'CHASE' || g.state === 'SCATTER' || g.state === 'LEAVING_HOUSE') {
        state = 'DYING';
        audio.playDeath();
        pacman.deathProgress = 0.0;
        break;
      }
    }
  }
}

function update() {
  if (state === 'GHOST_PAUSE') {
    freezeTimer--;
    if (freezeTimer <= 0) state = 'PLAYING';
    return;
  }

  if (state === 'LEVEL_CLEAR') {
    freezeTimer--;
    if (freezeTimer <= 0) {
      clearFlashWhite = !clearFlashWhite;
      clearFlashCount++;
      freezeTimer = 10;
      if (clearFlashCount >= 8) {
        clearFlashWhite = false;
        clearFlashCount = 0;
        level++;
        initMaze();
        resetPositions();
        state = 'READY';
        freezeTimer = 180;
        audio.playIntro();
      }
    }
    return;
  }

  if (state === 'READY') {
    freezeTimer--;
    if (freezeTimer <= 0) {
      state = 'PLAYING';
      audio.setSiren(1);
    }
    return;
  }

  if (state === 'DYING') {
    pacman.deathProgress += 0.025;
    if (pacman.deathProgress >= 1.0) {
      lives--;
      if (lives <= 0) {
        state = 'GAME_OVER';
      } else {
        resetPositions();
        state = 'READY';
        freezeTimer = 120;
        audio.playIntro();
      }
    }
    return;
  }

  if (state !== 'PLAYING') return;

  updatePacman();
  updateGhosts();
  checkCollisions();

  if (fruitActive) {
    fruitTimer--;
    if (fruitTimer <= 0) fruitActive = false;
  }

  for (let i = floatingScores.length - 1; i >= 0; i--) {
    floatingScores[i].y -= 0.6;
    floatingScores[i].alpha -= 0.02;
    if (floatingScores[i].alpha <= 0) floatingScores.splice(i, 1);
  }
}

function render() {
  ctx.fillStyle = '#050814';
  ctx.fillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

  // Header
  ctx.font = 'bold 14px monospace';
  ctx.fillStyle = '#38bdf8';
  ctx.fillText('1UP SCORE', 24, 28);
  ctx.fillStyle = '#ffffff';
  ctx.font = 'bold 22px monospace';
  ctx.fillText(score.toString(), 24, 52);

  ctx.font = 'bold 14px monospace';
  ctx.fillStyle = '#ef4444';
  ctx.fillText('HIGH SCORE', WINDOW_WIDTH - 150, 28);
  ctx.fillStyle = '#ffffff';
  ctx.font = 'bold 22px monospace';
  ctx.fillText(highScore.toString(), WINDOW_WIDTH - 150, 52);

  // Maze
  const offsetY = HEADER_HEIGHT;
  const wallStroke = clearFlashWhite ? '#ffffff' : '#2563eb';
  const wallFill = clearFlashWhite ? '#f0f0ff' : '#0a1128';

  for (let r = 0; r < ROWS; r++) {
    for (let c = 0; c < COLS; c++) {
      const cell = maze[r][c];
      const x = c * TILE_SIZE;
      const y = offsetY + r * TILE_SIZE;

      if (cell === 1) {
        ctx.fillStyle = wallFill;
        ctx.fillRect(x + 2, y + 2, TILE_SIZE - 4, TILE_SIZE - 4);
        ctx.strokeStyle = wallStroke;
        ctx.lineWidth = 2;
        ctx.strokeRect(x + 1, y + 1, TILE_SIZE - 2, TILE_SIZE - 2);
      } else if (cell === 4) {
        ctx.fillStyle = '#fb7185';
        ctx.fillRect(x, y + TILE_SIZE / 2 - 2, TILE_SIZE, 4);
      } else if (cell === 2) {
        ctx.fillStyle = '#fed7aa';
        ctx.beginPath();
        ctx.arc(x + 12, y + 12, 3, 0, Math.PI * 2);
        ctx.fill();
      } else if (cell === 3) {
        const pulse = (Math.sin(Date.now() * 0.008) + 1.0) * 0.5;
        ctx.fillStyle = '#fef08a';
        ctx.beginPath();
        ctx.arc(x + 12, y + 12, 5 + pulse * 3, 0, Math.PI * 2);
        ctx.fill();
      }
    }
  }

  // Fruit
  if (fruitActive) {
    const fx = 13.5 * TILE_SIZE;
    const fy = offsetY + 17.5 * TILE_SIZE;
    ctx.fillStyle = STAGE_FRUITS[fruitIndex].color;
    ctx.beginPath();
    ctx.arc(fx, fy, 9, 0, Math.PI * 2);
    ctx.fill();
    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 9px monospace';
    ctx.fillText(STAGE_FRUITS[fruitIndex].name.substr(0, 2), fx - 6, fy + 3);
  }

  // Draw Pac-Man
  if (state !== 'LEVEL_CLEAR') {
    const px = pacman.x;
    const py = offsetY + pacman.y;
    const rad = TILE_SIZE * 0.46;

    ctx.fillStyle = '#facc15';
    ctx.beginPath();

    let startAngle = 0, endAngle = Math.PI * 2;
    if (state === 'DYING') {
      const dissolve = pacman.deathProgress * Math.PI;
      let rot = 0;
      if (pacman.dir === 'DOWN') rot = Math.PI / 2;
      else if (pacman.dir === 'LEFT') rot = Math.PI;
      else if (pacman.dir === 'UP') rot = -Math.PI / 2;
      startAngle = rot + dissolve;
      endAngle = rot + (Math.PI * 2) - dissolve;
    } else {
      let rot = 0;
      if (pacman.dir === 'DOWN') rot = Math.PI / 2;
      else if (pacman.dir === 'LEFT') rot = Math.PI;
      else if (pacman.dir === 'UP') rot = -Math.PI / 2;
      const mouth = pacman.mouthTan * 0.9;
      startAngle = rot + mouth;
      endAngle = rot + (Math.PI * 2) - mouth;
    }

    ctx.moveTo(px, py);
    ctx.arc(px, py, rad, startAngle, endAngle);
    ctx.closePath();
    ctx.fill();
  }

  // Draw Ghosts
  if (state !== 'DYING' && state !== 'LEVEL_CLEAR') {
    ghosts.forEach(g => {
      const gx = g.x;
      const gy = offsetY + g.y;
      const rad = TILE_SIZE * 0.46;

      if (g.state === 'EATEN') {
        ctx.fillStyle = '#ffffff';
        ctx.beginPath();
        ctx.arc(gx - 4, gy - 2, 4, 0, Math.PI * 2);
        ctx.arc(gx + 4, gy - 2, 4, 0, Math.PI * 2);
        ctx.fill();

        let ox = g.dir === 'RIGHT' ? 2 : (g.dir === 'LEFT' ? -2 : 0);
        let oy = g.dir === 'DOWN' ? 2 : (g.dir === 'UP' ? -2 : 0);
        ctx.fillStyle = '#1e3a8a';
        ctx.beginPath();
        ctx.arc(gx - 4 + ox, gy - 2 + oy, 2, 0, Math.PI * 2);
        ctx.arc(gx + 4 + ox, gy - 2 + oy, 2, 0, Math.PI * 2);
        ctx.fill();
        return;
      }

      let col = g.color;
      if (g.state === 'FRIGHTENED') {
        if (frightenedTimer < 150 && Math.floor(frightenedTimer / 12) % 2 === 0) col = '#ffffff';
        else col = '#1e40af';
      }

      ctx.fillStyle = col;
      // Head
      ctx.beginPath();
      ctx.arc(gx, gy - 2, rad, Math.PI, 0, false);
      // Skirt
      ctx.lineTo(gx + rad, gy + rad - 2);
      // Ruffles
      const wave = Math.sin(Date.now() * 0.015) * 2.5;
      ctx.lineTo(gx + rad * 0.5, gy + rad + wave);
      ctx.lineTo(gx, gy + rad - wave);
      ctx.lineTo(gx - rad * 0.5, gy + rad + wave);
      ctx.lineTo(gx - rad, gy + rad - 2);
      ctx.closePath();
      ctx.fill();

      // Eyes
      if (g.state === 'FRIGHTENED') {
        ctx.fillStyle = '#fed7aa';
        ctx.beginPath();
        ctx.arc(gx - 4, gy - 3, 2, 0, Math.PI * 2);
        ctx.arc(gx + 4, gy - 3, 2, 0, Math.PI * 2);
        ctx.fill();
      } else {
        let ox = g.dir === 'RIGHT' ? 2 : (g.dir === 'LEFT' ? -2 : 0);
        let oy = g.dir === 'DOWN' ? 2 : (g.dir === 'UP' ? -2 : 0);
        ctx.fillStyle = '#ffffff';
        ctx.beginPath();
        ctx.arc(gx - 4, gy - 3, 4, 0, Math.PI * 2);
        ctx.arc(gx + 4, gy - 3, 4, 0, Math.PI * 2);
        ctx.fill();
        ctx.fillStyle = '#1e3a8a';
        ctx.beginPath();
        ctx.arc(gx - 4 + ox, gy - 3 + oy, 2, 0, Math.PI * 2);
        ctx.arc(gx + 4 + ox, gy - 3 + oy, 2, 0, Math.PI * 2);
        ctx.fill();
      }
    });
  }

  // Floating points
  floatingScores.forEach(fs => {
    ctx.fillStyle = `rgba(56, 189, 248, ${Math.max(0, fs.alpha)})`;
    ctx.font = 'bold 14px monospace';
    ctx.fillText(fs.text, fs.x - 12, offsetY + fs.y);
  });

  // Footer
  const footerY = WINDOW_HEIGHT - FOOTER_HEIGHT + 16;
  ctx.fillStyle = '#94a3b8';
  ctx.font = 'bold 14px monospace';
  ctx.fillText('LIVES:', 24, footerY + 8);
  for (let i = 0; i < lives - 1; i++) {
    ctx.fillStyle = '#facc15';
    ctx.beginPath();
    ctx.arc(95 + i * 24, footerY + 4, 8, 0.2 * Math.PI, 1.8 * Math.PI);
    ctx.lineTo(95 + i * 24, footerY + 4);
    ctx.fill();
  }

  ctx.fillStyle = '#22c55e';
  ctx.fillText(`LVL ${level}`, WINDOW_WIDTH - 200, footerY + 8);

  // Overlays
  if (state === 'START_SCREEN') {
    ctx.fillStyle = 'rgba(3, 7, 18, 0.9)';
    ctx.fillRect(WINDOW_WIDTH / 2 - 220, WINDOW_HEIGHT / 2 - 160, 440, 290);
    ctx.strokeStyle = '#3b82f6';
    ctx.lineWidth = 2;
    ctx.strokeRect(WINDOW_WIDTH / 2 - 220, WINDOW_HEIGHT / 2 - 160, 440, 290);

    ctx.fillStyle = '#facc15';
    ctx.font = 'bold 36px monospace';
    ctx.textAlign = 'center';
    ctx.fillText('PAC-MAN', WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 100);

    ctx.fillStyle = '#38bdf8';
    ctx.font = 'bold 12px monospace';
    ctx.fillText('DEVELOPER: MD. ABU RISE ZUNAED', WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 65);

    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 16px monospace';
    ctx.fillText('PRESS SPACE OR TAP SCREEN', WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 15);
    ctx.fillText('TO START GAME', WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 15);

    ctx.fillStyle = '#94a3b8';
    ctx.font = '12px monospace';
    ctx.fillText('ARROWS / WASD / TOUCH CONTROLS', WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 75);
    ctx.textAlign = 'left';
  } else if (state === 'READY') {
    ctx.fillStyle = '#fde047';
    ctx.font = 'bold 24px monospace';
    ctx.textAlign = 'center';
    ctx.fillText('READY!', WINDOW_WIDTH / 2, offsetY + 17 * TILE_SIZE + 10);
    ctx.textAlign = 'left';
  } else if (state === 'PAUSED') {
    ctx.fillStyle = '#38bdf8';
    ctx.font = 'bold 28px monospace';
    ctx.textAlign = 'center';
    ctx.fillText('PAUSED', WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    ctx.textAlign = 'left';
  } else if (state === 'GAME_OVER') {
    ctx.fillStyle = 'rgba(3, 7, 18, 0.9)';
    ctx.fillRect(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 120, 400, 230);
    ctx.strokeStyle = '#ef4444';
    ctx.lineWidth = 2;
    ctx.strokeRect(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 120, 400, 230);

    ctx.fillStyle = '#ef4444';
    ctx.font = 'bold 30px monospace';
    ctx.textAlign = 'center';
    ctx.fillText('GAME OVER', WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 60);

    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 18px monospace';
    ctx.fillText(`FINAL SCORE: ${score}`, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 15);

    ctx.fillStyle = '#facc15';
    ctx.font = 'bold 14px monospace';
    ctx.fillText('TAP OR PRESS SPACE TO PLAY AGAIN', WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 35);

    ctx.fillStyle = '#38bdf8';
    ctx.font = '11px monospace';
    ctx.fillText('DEV: MD. ABU RISE ZUNAED', WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 75);
    ctx.textAlign = 'left';
  }
}

// Game Loop (Solid 60 FPS)
let lastTimestamp = 0;
function loop(timestamp) {
  requestAnimationFrame(loop);
  if (!lastTimestamp) lastTimestamp = timestamp;
  const elapsed = timestamp - lastTimestamp;
  if (elapsed >= 15.5) {
    lastTimestamp = timestamp;
    update();
    render();
  }
}

// User Input Listeners
window.addEventListener('keydown', (e) => {
  audio.ensureContext();
  switch (e.key) {
    case 'ArrowUp': case 'w': case 'W': pacman.nextDir = 'UP'; break;
    case 'ArrowDown': case 's': case 'S': pacman.nextDir = 'DOWN'; break;
    case 'ArrowLeft': case 'a': case 'A': pacman.nextDir = 'LEFT'; break;
    case 'ArrowRight': case 'd': case 'D': pacman.nextDir = 'RIGHT'; break;
    case 'p': case 'P':
      if (state === 'PLAYING') { state = 'PAUSED'; audio.setSiren(0); }
      else if (state === 'PAUSED') { state = 'PLAYING'; audio.setSiren(frightenedTimer > 0 ? 2 : 1); }
      break;
    case 'm': case 'M':
      const muted = audio.toggleMute();
      document.getElementById('btnSound').textContent = muted ? '🔇 Muted' : '🔊 Sound';
      break;
    case ' ': case 'Enter':
      if (state === 'START_SCREEN' || state === 'GAME_OVER') startNewGame();
      else if (state === 'PAUSED') { state = 'PLAYING'; audio.setSiren(frightenedTimer > 0 ? 2 : 1); }
      break;
  }
});

// Touch and Button Controls for Mobile (iPhone, iPad, Android)
document.getElementById('btnUp').onclick = () => { audio.ensureContext(); pacman.nextDir = 'UP'; };
document.getElementById('btnDown').onclick = () => { audio.ensureContext(); pacman.nextDir = 'DOWN'; };
document.getElementById('btnLeft').onclick = () => { audio.ensureContext(); pacman.nextDir = 'LEFT'; };
document.getElementById('btnRight').onclick = () => { audio.ensureContext(); pacman.nextDir = 'RIGHT'; };
document.getElementById('btnStart').onclick = () => {
  audio.ensureContext();
  if (state === 'START_SCREEN' || state === 'GAME_OVER') startNewGame();
  else if (state === 'PAUSED') { state = 'PLAYING'; audio.setSiren(frightenedTimer > 0 ? 2 : 1); }
};
document.getElementById('btnPause').onclick = () => {
  audio.ensureContext();
  if (state === 'PLAYING') { state = 'PAUSED'; audio.setSiren(0); }
  else if (state === 'PAUSED') { state = 'PLAYING'; audio.setSiren(frightenedTimer > 0 ? 2 : 1); }
};
document.getElementById('btnSound').onclick = () => {
  audio.ensureContext();
  const muted = audio.toggleMute();
  document.getElementById('btnSound').textContent = muted ? '🔇 Muted' : '🔊 Sound';
};

// Canvas Swipe Gesture for Touch Devices
let touchStartX = 0, touchStartY = 0;
canvas.addEventListener('touchstart', (e) => {
  audio.ensureContext();
  if (state === 'START_SCREEN' || state === 'GAME_OVER') {
    startNewGame();
    return;
  }
  const touch = e.touches[0];
  touchStartX = touch.clientX;
  touchStartY = touch.clientY;
}, { passive: false });

canvas.addEventListener('touchmove', (e) => {
  e.preventDefault();
}, { passive: false });

canvas.addEventListener('touchend', (e) => {
  if (state !== 'PLAYING') return;
  const touch = e.changedTouches[0];
  const dx = touch.clientX - touchStartX;
  const dy = touch.clientY - touchStartY;
  if (Math.abs(dx) > 20 || Math.abs(dy) > 20) {
    if (Math.abs(dx) > Math.abs(dy)) {
      pacman.nextDir = dx > 0 ? 'RIGHT' : 'LEFT';
    } else {
      pacman.nextDir = dy > 0 ? 'DOWN' : 'UP';
    }
  }
});

// Initialize & Launch
initMaze();
requestAnimationFrame(loop);
