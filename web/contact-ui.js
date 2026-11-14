// Coupled contact diagnostic. All dynamics run in the compiled C++ Bullet module
// using original CONFIGFILE, CARSUSPENSION and CARTIRE implementations.
window.initFayawoContacts=async function(module){
 const el=id=>document.getElementById(id);
 const status=el('contact-status'),carSelect=el('contact-car'),tireSelect=el('contact-tire');
 const time=el('contact-time'),throttle=el('contact-throttle'),steer=el('contact-steer');
 const loadCar=module.cwrap('fayawo_contact_load_car','number',['string']);
 const loadTire=module.cwrap('fayawo_contact_load_tire','number',['string']);
 const asset=async path=>{
  const r=await fetch('./'+path);
  if(!r.ok)throw Error('Missing original asset '+path);
  return r.text();
 };
 const response=await fetch('./assets/manifest.json');
 if(!response.ok)throw Error('Original car manifest unavailable');
 const manifest=await response.json();
 carSelect.replaceChildren();tireSelect.replaceChildren();
 manifest.cars.forEach(car=>{const option=document.createElement('option');option.value=car.config;option.textContent=car.id;carSelect.append(option)});
 (manifest.tires||[]).forEach(name=>{const option=document.createElement('option');option.value='assets/tires/'+name;option.textContent=name;tireSelect.append(option)});
 if(!carSelect.options.length||!tireSelect.options.length)throw Error('Original vehicle/tyre data missing');
 carSelect.value=(manifest.cars.find(c=>c.id==='ES')||manifest.cars[0]).config;
 tireSelect.value='assets/tires/asphalt.tire';
 if(!tireSelect.value)tireSelect.selectedIndex=0;
 carSelect.disabled=tireSelect.disabled=false;
 let lastAsset='';
 async function updateAsset(){
  const key=carSelect.value+'|'+tireSelect.value;
  if(key===lastAsset)return;
  if(loadCar(await asset(carSelect.value))!==1)throw Error('Original C++ car parser failed');
  if(loadTire(await asset(tireSelect.value))!==1)throw Error('Original C++ tyre parser failed');
  lastAsset=key;
 }
 async function simulate(){
  status.textContent='Calculating native contact dynamics…';
  await updateAsset();
  if(module._fayawo_contact_reset()!==1)throw Error('Native Bullet contact world reset failed');
  const duration=+time.value,pedal=+throttle.value,steering=+steer.value;
  el('contact-time-value').textContent=duration.toFixed(2);
  el('contact-throttle-value').textContent=(pedal*100).toFixed(0)+'%';
  el('contact-steer-value').textContent=steering.toFixed(2);
  const n=Math.round(duration*120),values=[{x:0,y:1}];
  let maxContacts=0,normal=0,traction=0;
  for(let i=0;i<n;i++){
   if(module._fayawo_contact_step(1/120,pedal,steering)!==1)throw Error('Bullet contact step failed');
   maxContacts=Math.max(maxContacts,module._fayawo_contact_count());
   normal=module._fayawo_contact_normal();
   traction=module._fayawo_contact_traction();
   if(i%5===4||i===n-1)values.push({x:module._fayawo_contact_z(),y:module._fayawo_contact_y()});
  }
  const pos=module._fayawo_contact_z(),height=module._fayawo_contact_y();
  el('contact-position').textContent=pos.toFixed(2)+' m';
  el('contact-height').textContent=height.toFixed(3)+' m';
  el('contact-speed').textContent=module._fayawo_contact_vz().toFixed(2)+' m/s';
  el('contact-mass').textContent=module._fayawo_contact_mass().toFixed(0)+' kg';
  el('contact-wheels').textContent=module._fayawo_contact_count()+' / 4 (up to '+maxContacts+')';
  el('contact-load').textContent=normal.toFixed(0)+' N';
  el('contact-drive').textContent=traction.toFixed(0)+' N';
  const canvas=el('contact-chart'),ctx=canvas.getContext('2d');
  if(ctx){
   const w=canvas.width,h=canvas.height,p=40;
   ctx.clearRect(0,0,w,h);
   const minX=Math.min(0,...values.map(v=>v.x))-1,maxX=Math.max(1,...values.map(v=>v.x))+1;
   const minY=Math.min(0,...values.map(v=>v.y))-.2,maxY=Math.max(1.5,...values.map(v=>v.y))+.2;
   const tx=x=>p+(x-minX)/(maxX-minX)*(w-2*p);
   const ty=y=>h-p-(y-minY)/(maxY-minY)*(h-2*p);
   ctx.strokeStyle='#55695e';ctx.lineWidth=1;ctx.beginPath();ctx.moveTo(p,ty(0));ctx.lineTo(w-p,ty(0));ctx.stroke();
   ctx.strokeStyle='#d4f2b5';ctx.lineWidth=2;ctx.beginPath();
   values.forEach((point,i)=>{if(i===0)ctx.moveTo(tx(point.x),ty(point.y));else ctx.lineTo(tx(point.x),ty(point.y))});ctx.stroke();
   ctx.fillStyle='#d4f2b5';ctx.beginPath();ctx.arc(tx(pos),ty(height),5,0,Math.PI*2);ctx.fill();
   ctx.fillStyle='#a2b6a8';ctx.font='12px Arial';ctx.fillText('Bullet chassis + real VDrift tyre and spring forces',p,20);
  }
  status.textContent='Original C++ wheel-contact and Bullet simulation loaded';
 }
 let version=0;
 async function refresh(){
  const current=++version;
  try{await simulate()}catch(err){if(current===version){status.textContent=err.message;console.error(err)}}
 }
 [time,throttle,steer].forEach(input=>input.addEventListener('input',refresh));
 [carSelect,tireSelect].forEach(input=>input.addEventListener('change',refresh));
 await refresh();
};
