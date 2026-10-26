// ═══════════════════════════════════════════════════════════════════
//  OrnitoSIRT · panel de control web (se sirve en http://192.168.4.1)
//  Colores y tipografía de la web corporativa de SIRT.
// ═══════════════════════════════════════════════════════════════════
#pragma once
#include <pgmspace.h>

const char PAGINA_HTML[] PROGMEM = R"html(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#070F26">
<title>OrnitoSIRT</title>
<style>
:root{--marino:#070F26;--noche:#00191D;--t1:#111F42;--t2:#0C1733;--borde:#1E2D52;--lima:#C6D32B;--verde:#2AB17E;
--petroleo:#076471;--texto:#E6E7E9;--tenue:#8C9AB3;--alerta:#F25C3B;
--grad:linear-gradient(90deg,#C1D42F 0%,#2AB17E 52%,#076471 100%)}
*{box-sizing:border-box;margin:0;padding:0;-webkit-tap-highlight-color:transparent}
html{background:var(--marino)}
body{font-family:Inter,system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;color:var(--texto);min-height:100vh;
background:radial-gradient(120% 55% at 50% -12%,#0B3A4A 0%,var(--marino) 62%);padding:16px 16px 40px;
-webkit-font-smoothing:antialiased}
main{max-width:460px;margin:0 auto;display:flex;flex-direction:column;gap:14px}
header{display:flex;align-items:center;justify-content:space-between;gap:10px;padding:4px 2px}
.marca{display:flex;align-items:center;gap:12px}
.marca svg{height:24px;width:auto;display:block}
.marca .sep{width:1px;height:24px;background:var(--borde)}
.marca h1{font-size:20px;font-weight:300;letter-spacing:.2px}
.marca h1 b{font-weight:600}
.chip{font-size:12px;color:var(--tenue);display:flex;align-items:center;gap:6px;background:var(--t2);
border:1px solid var(--borde);border-radius:99px;padding:5px 10px;white-space:nowrap;max-width:46%;overflow:hidden}
.chip i{width:7px;height:7px;border-radius:50%;background:var(--verde);box-shadow:0 0 8px var(--verde);flex:none}
.chip.off i{background:var(--alerta);box-shadow:0 0 8px var(--alerta)}
.card{background:linear-gradient(180deg,var(--t1),var(--t2));border:1px solid var(--borde);border-radius:18px;padding:14px}
.card h2{font-size:11px;font-weight:600;letter-spacing:1.6px;text-transform:uppercase;color:var(--tenue);
margin:0 2px 12px;display:flex;justify-content:space-between;align-items:baseline;gap:8px}
.card h2 em{font-style:normal;text-transform:none;letter-spacing:0;font-weight:400}
.grid{display:grid;grid-template-columns:repeat(4,1fr);gap:8px}
.cara{background:rgba(255,255,255,.03);border:1px solid var(--borde);border-radius:14px;padding:10px 2px 8px;
color:var(--texto);font:inherit;cursor:pointer;display:flex;flex-direction:column;align-items:center;gap:5px;
transition:transform .1s,border-color .15s,background .15s}
.cara span{font-size:24px;line-height:1}
.cara small{font-size:10.5px;color:var(--tenue)}
.cara:active,.btn:active,.demo:active{transform:scale(.95)}
.cara.on{border-color:var(--lima);background:rgba(198,211,43,.10)}
.cara.on small{color:var(--lima)}
.fila{display:flex;gap:8px;flex-wrap:wrap;align-items:center}
.btn{flex:1;min-width:0;background:rgba(255,255,255,.04);border:1px solid var(--borde);border-radius:12px;
color:var(--texto);font:inherit;font-size:13px;font-weight:500;padding:11px 8px;cursor:pointer;
transition:transform .1s,background .15s;white-space:nowrap}
.btn.pri{background:var(--lima);border-color:var(--lima);color:var(--marino);font-weight:600}
.btn.sec{border-color:var(--verde);color:var(--verde)}
.btn.auto{flex:0 0 auto;padding:11px 18px}
.acc{flex:1 1 30%}
.demo{display:flex;align-items:center;gap:12px;width:100%;text-align:left;background:rgba(255,255,255,.03);
border:1px solid var(--borde);border-radius:14px;padding:11px 12px;color:var(--texto);font:inherit;cursor:pointer;
margin-bottom:8px;transition:transform .1s,border-color .15s}
.demo:last-child{margin-bottom:0}
.demo .ic{width:38px;height:38px;border-radius:11px;display:grid;place-items:center;font-size:19px;
background:rgba(42,177,126,.13);flex:none}
.demo b{display:block;font-size:14px;font-weight:600}
.demo small{color:var(--tenue);font-size:12px}
.demo .est{margin-left:auto;font-size:10.5px;letter-spacing:1px;color:var(--marino);background:var(--lima);
font-weight:700;border-radius:99px;padding:3px 8px;display:none}
.demo.on{border-color:var(--lima)}
.demo.on .est{display:block}
#pad{position:relative;width:100%;aspect-ratio:16/10;border-radius:14px;touch-action:none;overflow:hidden;
background:radial-gradient(circle at center,rgba(7,100,113,.38),rgba(0,25,29,.55));border:1px solid var(--borde);cursor:crosshair}
#pad:before,#pad:after{content:"";position:absolute;background:var(--borde)}
#pad:before{left:50%;top:0;bottom:0;width:1px}
#pad:after{top:50%;left:0;right:0;height:1px}
#punto{position:absolute;left:50%;top:50%;width:34px;height:34px;margin:-17px 0 0 -17px;border-radius:50%;
background:var(--grad);box-shadow:0 0 18px rgba(198,211,43,.45);transition:left .3s,top .3s;z-index:1}
#pad.activo #punto{transition:none}
input[type=text]{width:100%;background:var(--noche);border:1px solid var(--borde);border-radius:12px;
color:var(--texto);font:inherit;font-size:16px;padding:12px;outline:none}
input[type=text]:focus{border-color:var(--verde)}
.presets{display:flex;gap:6px;flex-wrap:wrap;margin:10px 0 12px}
.preset{background:none;border:1px dashed var(--borde);border-radius:99px;color:var(--tenue);font:inherit;
font-size:12px;padding:6px 10px;cursor:pointer}
.preset:active{border-color:var(--lima);color:var(--lima)}
.colores{display:flex;gap:8px;align-items:center;flex-wrap:wrap}
.sw{width:30px;height:30px;border-radius:50%;border:2px solid transparent;cursor:pointer;flex:none;padding:0;
box-shadow:inset 0 0 0 1px rgba(255,255,255,.18);outline:2px solid transparent;outline-offset:1px}
.sw.on{outline-color:var(--texto)}
input[type=color]{width:30px;height:30px;border:0;border-radius:50%;padding:0;background:none;cursor:pointer}
.ajuste{display:flex;align-items:center;justify-content:space-between;gap:12px;padding:11px 2px;
border-top:1px solid rgba(255,255,255,.05)}
.ajuste:first-of-type{border-top:0;padding-top:2px}
.ajuste .n{font-size:14px}
.ajuste small{display:block;font-size:11.5px;color:var(--tenue);margin-top:2px}
input[type=range]{flex:1;max-width:210px;accent-color:var(--lima)}
.seg{display:flex;background:var(--noche);border:1px solid var(--borde);border-radius:10px;padding:3px}
.seg button{background:none;border:0;color:var(--tenue);font:inherit;font-size:12px;padding:6px 10px;border-radius:7px;cursor:pointer}
.seg button.on{background:var(--lima);color:var(--marino);font-weight:600}
.tog{position:relative;width:46px;height:27px;flex:none}
.tog input{opacity:0;width:0;height:0}
.tog span{position:absolute;inset:0;background:var(--borde);border-radius:99px;transition:.2s;cursor:pointer}
.tog span:before{content:"";position:absolute;width:21px;height:21px;left:3px;top:3px;background:var(--texto);
border-radius:50%;transition:.2s}
.tog input:checked+span{background:var(--verde)}
.tog input:checked+span:before{transform:translateX(19px)}
.panel{display:none;margin-top:12px;flex-direction:column;gap:10px}
.panel.abierto{display:flex}
canvas{width:100%;aspect-ratio:1;border-radius:12px;border:1px solid var(--borde);touch-action:none;display:block}
.mono{font-family:ui-monospace,Menlo,Consolas,monospace}
.etq{font-size:11px;color:var(--tenue);min-width:44px}
footer{text-align:center;color:var(--tenue);font-size:11.5px;line-height:1.7;margin-top:4px}
footer .g{background:var(--grad);-webkit-background-clip:text;background-clip:text;color:transparent;font-weight:700}
#aviso{position:fixed;left:50%;bottom:22px;transform:translate(-50%,20px);background:var(--t1);
border:1px solid var(--verde);color:var(--texto);border-radius:12px;padding:10px 16px;font-size:13px;opacity:0;
transition:.2s;pointer-events:none;z-index:9;white-space:nowrap}
#aviso.ver{opacity:1;transform:translate(-50%,0)}
#aviso.mal{border-color:var(--alerta)}
</style>
</head>
<body>
<main>
<header>
  <div class="marca">
    <svg viewBox="0 0 299 90.4" aria-label="SIRT"><defs><linearGradient id="gl" x1="0" x2="1">
    <stop offset="0" stop-color="#C1D42F"/><stop offset=".52" stop-color="#2AB17E"/><stop offset="1" stop-color="#076471"/>
    </linearGradient></defs><path fill="url(#gl)" d="M120,11.3v67.8c0,4.6-3.8,8.4-8.4,8.4s-8.4-3.8-8.4-8.4V11.3l0,0c0-4.6,3.8-8.3,8.4-8.3S120,6.7,120,11.3L120,11.3z M162.2,3.1c-14,0-25.3,11.3-25.3,25.3v50.7c0,4.6,3.8,8.4,8.4,8.4c4.6,0,8.4-3.8,8.4-8.4V28.4c0-4.6,3.8-8.5,8.4-8.5h29.3c4.6,0,8.4-3.8,8.4-8.4S196,3,191.4,3L162.2,3.1 M288.6,3.1h-67.3c-4.6,0-8.4,3.8-8.4,8.4s3.8,8.4,8.4,8.4h16.9l0,0c4.6,0,8.4,3.8,8.4,8.4v50.8c0,4.6,3.8,8.4,8.4,8.4s8.4-3.8,8.4-8.4V28.3c0-4.6,3.8-8.4,8.4-8.4l0,0h16.9c4.6,0,8.4-3.8,8.4-8.4S293.2,3.1,288.6,3.1L288.6,3.1z M60.7,36.8L60.7,36.8L60.7,36.8H27.3c-4.7,0-8.5-3.8-8.5-8.5s3.8-8.5,8.5-8.5h49.2c4.6,0,8.4-3.8,8.4-8.4S81.1,3,76.5,3H27.3l0,0C13.3,3.1,2,14.5,2,28.4s11.3,25.3,25.3,25.3h34.1c4.7,0,8.5,3.8,8.5,8.5s-3.8,8.5-8.5,8.5H10.6c-4.6,0-8.4,3.8-8.4,8.4s3.8,8.4,8.4,8.4h50.1C74.6,87.4,86,76.1,86,62.1S74.7,36.8,60.7,36.8L60.7,36.8z"/></svg>
    <div class="sep"></div>
    <h1>Ornito<b>SIRT</b></h1>
  </div>
  <div class="chip" id="chip"><i></i><span id="estado">conectando…</span></div>
</header>

<section class="card">
  <h2>Expresiones</h2>
  <div class="grid" id="caras"></div>
</section>

<section class="card">
  <h2>Acciones</h2>
  <div class="fila">
    <button class="btn acc" onclick="accion('parpadeo')">😌 Parpadeo</button>
    <button class="btn acc" onclick="accion('guino')">😉 Guiño</button>
    <button class="btn acc" onclick="accion('salto')">🐦 Saltito</button>
    <button class="btn acc" onclick="accion('si')">👍 Sí</button>
    <button class="btn acc" onclick="accion('no')">🙅 No</button>
    <button class="btn acc" onclick="accion('mirar')">👀 Curiosear</button>
  </div>
</section>

<section class="card">
  <h2>Mirada <em>arrastra el punto</em></h2>
  <div id="pad"><div id="punto"></div></div>
</section>

<section class="card">
  <h2>Demos SIRT</h2>
  <button class="demo" id="dPres" onclick="presentacion()"><span class="ic">▶️</span>
    <span><b>Modo presentación</b><small>Logo, servicios, radar y caras en bucle</small></span><span class="est">ACTIVO</span></button>
  <button class="demo" onclick="demo('radar')"><span class="ic">📡</span>
    <span><b>Escanear la red</b><small>Radar del SOC buscando amenazas</small></span></button>
  <button class="demo" onclick="demo('ataque')"><span class="ic">🛡️</span>
    <span><b>Simular un ataque</b><small>Alerta → amenaza bloqueada</small></span></button>
  <button class="demo" onclick="demo('logo')"><span class="ic">✨</span>
    <span><b>Logo SIRT</b><small>Expertos en servicios tecnológicos avanzados</small></span></button>
  <button class="demo" onclick="demo('wifi')"><span class="ic">📶</span>
    <span><b>Mostrar QR del WiFi</b><small>Para que otros se conecten</small></span></button>
</section>

<section class="card">
  <h2>Mensaje en pantalla</h2>
  <input type="text" id="msg" maxlength="90" placeholder="Escribe un mensaje…" enterkeyhint="send">
  <div class="presets" id="presets">
    <button class="preset">¡Bienvenidos a SIRT!</button>
    <button class="preset">SOC 24x7</button>
    <button class="preset">Ciberseguridad · Infraestructura · Cloud</button>
    <button class="preset">¡Gracias por venir!</button>
  </div>
  <div class="fila"><div class="colores" id="colMsg"></div>
    <button class="btn pri auto" style="margin-left:auto" onclick="enviaMsg()">Mostrar</button></div>
</section>

<section class="card">
  <h2>Terminal</h2>
  <button class="btn sec" id="bTerm" style="width:100%" onclick="abreTerm()">&gt;_ Abrir terminal</button>
  <div class="panel" id="pTerm">
    <input type="text" id="tin" class="mono" placeholder="escribe 'ayuda' y pulsa intro"
      autocomplete="off" autocorrect="off" autocapitalize="off" spellcheck="false" enterkeyhint="enter">
    <div class="fila"><button class="btn sec" onclick="intro()">↵ Intro</button>
      <button class="btn" onclick="cierraTerm()">✕ Salir</button></div>
  </div>
</section>

<section class="card">
  <h2>Lienzo</h2>
  <button class="btn sec" id="bLienzo" style="width:100%" onclick="abreLienzo()">✏️ Dibujar en la pantalla</button>
  <div class="panel" id="pLienzo">
    <canvas id="cv" width="240" height="240"></canvas>
    <div class="fila"><span class="etq">Pluma</span><div class="colores" id="colPen"></div>
      <input type="color" id="penLibre" value="#ffffff" title="Otro color"></div>
    <div class="fila"><span class="etq">Fondo</span><div class="colores" id="colFondoL"></div>
      <div class="seg" id="grosor" style="margin-left:auto"><button data-g="2">fino</button>
      <button data-g="5" class="on">medio</button><button data-g="10">grueso</button></div></div>
    <div class="fila"><button class="btn" onclick="borra()">Borrar</button>
      <button class="btn pri" onclick="cierraLienzo()">Hecho</button></div>
  </div>
</section>

<section class="card">
  <h2>Ajustes</h2>
  <div class="ajuste"><span class="n">Brillo</span><input type="range" id="brillo" min="5" max="100"></div>
  <div class="ajuste"><span class="n">Velocidad</span>
    <div class="seg" id="vel"><button data-v="1">Lenta</button><button data-v="2">Normal</button><button data-v="3">Rápida</button></div></div>
  <div class="ajuste"><span class="n">Color de ojos</span><div class="colores" id="ojos"></div></div>
  <div class="ajuste"><span class="n">Fondo</span><div class="colores" id="fondo"></div></div>
  <div class="ajuste"><div><span class="n">Vida propia</span><small>Parpadea, curiosea y se duerme si nadie le hace caso</small></div>
    <label class="tog"><input type="checkbox" id="auto"><span></span></label></div>
  <div class="ajuste"><div><span class="n">Pantalla encendida</span></div>
    <label class="tog"><input type="checkbox" id="luz"><span></span></label></div>
  <div class="ajuste"><div><span class="n">Presentación al arrancar</span><small>Ideal para stands y eventos</small></div>
    <label class="tog"><input type="checkbox" id="presIni"><span></span></label></div>
  <div class="ajuste"><div><span class="n">Inversión de color</span><small>Cámbialo si los colores salen en negativo</small></div>
    <label class="tog"><input type="checkbox" id="inv"><span></span></label></div>
  <div class="ajuste"><div><span class="n">Orientación</span><small>Gira la imagen 90°</small></div>
    <button class="btn auto" onclick="ajusta('rot=sig')">⟳ Girar</button></div>
</section>

<footer>
  <div><span class="g">SIRT</span> · Expertos en servicios tecnológicos avanzados</div>
  <div id="pie">OrnitoSIRT</div>
  <div>Basado en clawd-mochi (licencia MIT)</div>
</footer>
</main>
<div id="aviso"></div>

<script>
const $=s=>document.querySelector(s),$$=s=>document.querySelectorAll(s);
// El orden debe coincidir con el enum Animo del firmware
const CARAS=[['🙂','Normal'],['😊','Feliz'],['😠','Enfadado'],['😢','Triste'],['😮','Sorpresa'],['🤨','Sospecha'],
['😪','Cansado'],['😍','Amor'],['🤩','Euforia'],['😵','Mareado'],['😴','Dormido'],['👀','Vigilante']];
const OJOS=[['linear-gradient(180deg,#C6D32B,#2AB17E)','Degradado SIRT'],['#C6D32B','Lima'],['#2AB17E','Verde'],['#F2F4F5','Blanco']];
const FONDOS=[['#070F26','Marino'],['#00191D','Noche'],['#000000','Negro']];
const MSG=[['linear-gradient(90deg,#C1D42F,#2AB17E,#076471)','Degradado SIRT'],['#C6D32B','Lima'],['#2AB17E','Verde'],['#E6E7E9','Blanco']];
const PLUMAS=['#C6D32B','#2AB17E','#076471','#E6E7E9','#F25C3B','#070F26'];
const FONDOS_L=['#070F26','#E6E7E9','#000000','#076471'];
const ESCENAS={arranque:'arrancando',wifi:'mostrando QR',logo:'logo SIRT',servicio:'servicios',radar:'escaneando',
ataque:'¡alerta!',texto:'mensaje',terminal:'terminal',lienzo:'lienzo'};
let E={},tA,colMsg=0,pluma=PLUMAS[0],fondoL=FONDOS_L[0],grosor=5;

function aviso(t,mal){const a=$('#aviso');a.textContent=t;a.classList.toggle('mal',!!mal);a.classList.add('ver');
clearTimeout(tA);tA=setTimeout(()=>a.classList.remove('ver'),1700)}
async function api(r){try{const x=await fetch(r,{cache:'no-store'});if(!x.ok)throw 0;const j=await x.json();pinta(j);return j}
catch(e){$('#chip').classList.add('off');$('#estado').textContent='sin conexión';return null}}
function pinta(j){E=j;$('#chip').classList.remove('off');
 $('#estado').textContent=j.dormido?'durmiendo 💤':j.pres?'presentación':j.esc==='cara'?CARAS[j.animo][1].toLowerCase():(ESCENAS[j.esc]||j.esc);
 $$('.cara').forEach((b,i)=>b.classList.toggle('on',j.esc==='cara'&&!j.pres&&i===j.animo));
 $('#dPres').classList.toggle('on',!!j.pres);
 const br=$('#brillo');if(document.activeElement!==br)br.value=j.brillo;
 $$('#vel button').forEach(b=>b.classList.toggle('on',+b.dataset.v===j.vel));
 marca('#ojos',j.ojos);marca('#fondo',j.fondo);
 $('#auto').checked=!!j.auto;$('#luz').checked=!!j.luz;$('#presIni').checked=!!j.presIni;$('#inv').checked=!!j.inv;
 panel('pTerm','bTerm',j.esc==='terminal');panel('pLienzo','bLienzo',j.esc==='lienzo');
 $('#pie').textContent='OrnitoSIRT v'+j.ver+' · WiFi '+j.wifi+' · '+j.cli+(j.cli==1?' dispositivo':' dispositivos');
}
function panel(p,b,abierto){$('#'+p).classList.toggle('abierto',abierto);$('#'+b).style.display=abierto?'none':''}
function bolas(sel,lista,fn){const c=$(sel);lista.forEach((it,i)=>{const[col,nom]=Array.isArray(it)?it:[it,it];
 const b=document.createElement('button');b.className='sw';b.style.background=col;b.title=nom;
 b.onclick=()=>{fn(i,col);};c.appendChild(b)})}
function marca(sel,i){$$(sel+' .sw').forEach((b,k)=>b.classList.toggle('on',k===i))}

// Expresiones y acciones
CARAS.forEach(([e,n],i)=>{const b=document.createElement('button');b.className='cara';
 b.innerHTML='<span>'+e+'</span><small>'+n+'</small>';b.onclick=()=>api('/api/cara?a='+i);$('#caras').appendChild(b)});
function accion(a){api('/api/accion?a='+a)}
function demo(d){api('/api/demo?d='+d)}
function presentacion(){api('/api/demo?d=presentacion&on='+(E.pres?0:1))}

// Mirada: se envía siempre la última posición, sin acumular peticiones
const pad=$('#pad'),punto=$('#punto');let enVuelo=false,pend=null,activo=false;
function envMirada(){if(enVuelo||!pend)return;const p=pend;pend=null;enVuelo=true;
 fetch(p).catch(()=>{}).finally(()=>{enVuelo=false;envMirada()})}
function mueve(e){const r=pad.getBoundingClientRect();
 const x=Math.min(1,Math.max(0,(e.clientX-r.left)/r.width)),y=Math.min(1,Math.max(0,(e.clientY-r.top)/r.height));
 punto.style.left=x*100+'%';punto.style.top=y*100+'%';
 pend='/api/mirar?x='+Math.round(x*200-100)+'&y='+Math.round(y*200-100);envMirada()}
pad.addEventListener('pointerdown',e=>{activo=true;pad.classList.add('activo');pad.setPointerCapture(e.pointerId);mueve(e)});
pad.addEventListener('pointermove',e=>{if(activo)mueve(e)});
const suelta=()=>{if(!activo)return;activo=false;pad.classList.remove('activo');
 punto.style.left='50%';punto.style.top='50%';pend='/api/mirar?soltar=1';envMirada()};
pad.addEventListener('pointerup',suelta);pad.addEventListener('pointercancel',suelta);

// Mensaje
bolas('#colMsg',MSG,i=>{colMsg=i;marca('#colMsg',i)});marca('#colMsg',0);
async function enviaMsg(){const t=$('#msg').value.trim();if(!t){aviso('Escribe algo primero',1);return}
 if(await api('/api/texto?c='+colMsg+'&t='+encodeURIComponent(t)))aviso('Mensaje en pantalla')}
$$('.preset').forEach(b=>b.onclick=()=>{$('#msg').value=b.textContent;enviaMsg()});
$('#msg').addEventListener('keydown',e=>{if(e.key==='Enter'){e.preventDefault();enviaMsg()}});

// Terminal: se envía solo lo que cambia (funciona también con autocorrector)
const tin=$('#tin');let prev='',colaT=Promise.resolve();
function envT(q){colaT=colaT.then(()=>fetch('/api/terminal?'+q).catch(()=>{}))}
tin.addEventListener('input',()=>{const cur=tin.value;let i=0;
 while(i<cur.length&&i<prev.length&&cur[i]===prev[i])i++;
 const borra=[...prev.slice(i)].length,nuevo=cur.slice(i);
 if(borra||nuevo)envT('b='+borra+'&c='+encodeURIComponent(nuevo));prev=cur});
function intro(){envT('enter=1');tin.value='';prev='';tin.focus()}
tin.addEventListener('keydown',e=>{if(e.key==='Enter'){e.preventDefault();intro()}});
async function abreTerm(){await api('/api/terminal?abrir=1');tin.value='';prev='';tin.focus()}
function cierraTerm(){api('/api/terminal?cerrar=1')}

// Lienzo: cada trazo se envía al levantar el dedo (o cada 60 puntos)
const cv=$('#cv'),cx=cv.getContext('2d');let dib=false,trazo=[],colaL=Promise.resolve();
function pos(e){const r=cv.getBoundingClientRect();return{x:Math.round((e.clientX-r.left)*240/r.width),y:Math.round((e.clientY-r.top)*240/r.height)}}
function limpiaCv(){cx.fillStyle=fondoL;cx.fillRect(0,0,240,240)}
function envTrazo(t){if(!t.length)return;
 const q='/api/lienzo/trazo?pen='+pluma.slice(1)+'&g='+grosor+'&pts='+t.map(p=>p.x+','+p.y).join(';');
 colaL=colaL.then(()=>fetch(q).catch(()=>{}))}
cv.addEventListener('pointerdown',e=>{dib=true;cv.setPointerCapture(e.pointerId);const p=pos(e);trazo=[p];
 cx.fillStyle=pluma;cx.beginPath();cx.arc(p.x,p.y,grosor/2,0,7);cx.fill()});
cv.addEventListener('pointermove',e=>{if(!dib)return;const p=pos(e),u=trazo[trazo.length-1];
 if(Math.abs(p.x-u.x)+Math.abs(p.y-u.y)<3)return;
 cx.strokeStyle=pluma;cx.lineWidth=grosor;cx.lineCap='round';cx.beginPath();cx.moveTo(u.x,u.y);cx.lineTo(p.x,p.y);cx.stroke();
 trazo.push(p);if(trazo.length>=60){envTrazo(trazo);trazo=[p]}});
const finTrazo=()=>{if(!dib)return;dib=false;envTrazo(trazo);trazo=[]};
cv.addEventListener('pointerup',finTrazo);cv.addEventListener('pointercancel',finTrazo);
bolas('#colPen',PLUMAS,(i,c)=>{pluma=c;marca('#colPen',i)});marca('#colPen',0);
$('#penLibre').addEventListener('input',e=>{pluma=e.target.value;marca('#colPen',-1)});
bolas('#colFondoL',FONDOS_L,(i,c)=>{fondoL=c;marca('#colFondoL',i);borra()});marca('#colFondoL',0);
$$('#grosor button').forEach(b=>b.onclick=()=>{grosor=+b.dataset.g;$$('#grosor button').forEach(x=>x.classList.toggle('on',x===b))});
async function abreLienzo(){limpiaCv();await api('/api/lienzo?abrir=1&bg='+fondoL.slice(1))}
function borra(){limpiaCv();api('/api/lienzo?borrar=1&bg='+fondoL.slice(1))}
function cierraLienzo(){api('/api/lienzo?cerrar=1')}

// Ajustes
function ajusta(q){return api('/api/ajustes?'+q)}
let tB;$('#brillo').addEventListener('input',e=>{clearTimeout(tB);tB=setTimeout(()=>ajusta('brillo='+e.target.value),80)});
$$('#vel button').forEach(b=>b.onclick=()=>ajusta('vel='+b.dataset.v));
bolas('#ojos',OJOS,i=>ajusta('ojos='+i));bolas('#fondo',FONDOS,i=>ajusta('fondo='+i));
[['auto','auto'],['luz','luz'],['presIni','pres'],['inv','inv']].forEach(([id,k])=>
 $('#'+id).addEventListener('change',e=>ajusta(k+'='+(e.target.checked?1:0))));

api('/api/estado');
setInterval(()=>{if(!document.hidden)api('/api/estado')},3000);
</script>
</body>
</html>
)html";
