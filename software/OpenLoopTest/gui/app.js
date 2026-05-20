// ═══ Smart BLDC Servo – Open Loop Dashboard JS ═══
let ws = null;
let isKilled = true;
let lastRps = 0;
const CIRC = 326.7; // 2*PI*52 for gauges

function connectWS() {
    const ip = document.getElementById('ip-input').value.trim();
    if (ws) { ws.onclose = null; ws.close(); }
    ws = new WebSocket('ws://' + ip + ':81');
    ws.onopen = () => {
        document.getElementById('conn-dot').classList.add('active');
        document.getElementById('conn-text').textContent = 'Verbunden (' + ip + ')';
    };
    ws.onclose = () => {
        document.getElementById('conn-dot').classList.remove('active');
        document.getElementById('conn-text').textContent = 'Getrennt – Reconnect...';
        setTimeout(connectWS, 3000);
    };
    ws.onerror = () => {};
    ws.onmessage = (evt) => {
        try {
            const d = JSON.parse(evt.data);
            // Simulated Position (0-4096)
            if (d.pos !== undefined) {
                const deg = (d.pos / 4096 * 360).toFixed(0);
                document.getElementById('val-angle').textContent = deg;
                setGaugeRing('gauge-pos-ring', d.pos / 4096);
            }
            // RPS
            if (d.rps !== undefined) {
                lastRps = parseFloat(d.rps);
                document.getElementById('val-rps').textContent = lastRps.toFixed(2);
                setGaugeRing('gauge-rps-ring', Math.min(Math.abs(lastRps) / 5, 1)); // scale to 5 rps max
            }
            
            if (d.sp !== undefined) document.getElementById('val-setpoint').textContent = Math.round(d.sp);
            
            // Driver status
            if (d.tmc !== undefined) {
                const el = document.getElementById('val-tmc');
                el.textContent = d.tmc; el.className = 'tele-value ' + (d.tmc === 'OK' ? 'tele-ok' : 'tele-err');
            }
            // Kill state
            if (d.k === 1 && !isKilled) { isKilled = true; updatePowerUI(); }
            else if (d.k === 0 && isKilled) { isKilled = false; updatePowerUI(); }
            
        } catch(e) {}
    };
}

function setGaugeRing(id, frac) {
    const el = document.getElementById(id);
    if (el) el.style.strokeDashoffset = CIRC * (1 - Math.max(0, Math.min(1, frac)));
}

function send(obj) { if (ws && ws.readyState === WebSocket.OPEN) ws.send(JSON.stringify(obj)); }

function updateSpeed(v) { 
    document.getElementById('lbl-spd').textContent = v; 
    send({val: parseInt(v)}); 
}

function snapZero() { 
    document.getElementById('slider-speed').value = 0; 
    updateSpeed(0); 
}

// ── Power ──
function togglePower() { 
    if (isKilled) { 
        send({cmd:'enable'}); 
        isKilled = false; 
    } else { 
        send({cmd:'kill'}); 
        isKilled = true; 
    } 
    updatePowerUI(); 
}

function updatePowerUI() {
    const btn = document.getElementById('btn-power');
    const lbl = document.getElementById('power-label');
    if (isKilled) { 
        btn.className = 'power-button power-off'; 
        lbl.textContent = 'MOTOR AKTIVIEREN'; 
    }
    else { 
        btn.className = 'power-button power-on'; 
        lbl.textContent = 'NOT-AUS (MOTOR AKTIV)'; 
    }
}

window.addEventListener('load', connectWS);
