// Diagnostics for the ORIGINAL VDrift RIGIDBODY integration used in CARDYNAMICS.
// No JavaScript physics. Every simulation step runs in compiled native C++.
window.initFayawoRigidBody = function(module){
 const el=(id)=>document.getElementById(id);
 const seconds=el('body-time'),force=el('body-force'),torque=el('body-torque');
 const draw=()=>{
  const mass=module._fayawo_original_vehicle_mass();
  if(!Number.isFinite(mass)||mass<=0)throw Error('No original vehicle mass data loaded');
  if(module._fayawo_original_body_reset(mass,10)!==1)throw Error('Original RIGIDBODY reset failed');
  const duration=+seconds.value,thrust=+force.value,yaw=+torque.value;
  const steps=Math.round(duration*120);
  const samples=[{x:0,z:10}];
  for(let i=0;i<steps;i++){
   if(module._fayawo_original_body_step(1/120,thrust,yaw)!==1)throw Error('Original RIGIDBODY step failed');
   if(i%4===3||i===steps-1)samples.push({x:module._fayawo_original_body_x(),z:module._fayawo_original_body_z()});
  }
  el('body-seconds').textContent=duration.toFixed(2);
  el('body-force-value').textContent=thrust.toFixed(0);
  el('body-torque-value').textContent=yaw.toFixed(0);
  el('body-mass').textContent=mass.toFixed(1)+' kg';
  el('body-position').textContent=module._fayawo_original_body_x().toFixed(2)+', '+module._fayawo_original_body_z().toFixed(2)+' m';
  el('body-velocity').textContent=module._fayawo_original_body_vx().toFixed(2)+', '+module._fayawo_original_body_vz().toFixed(2)+' m/s';
  el('body-yaw').textContent=module._fayawo_original_body_yaw_rate().toFixed(4)+' rad/s';
  const canvas=el('body-plot'),ctx=canvas.getContext('2d');
  if(ctx){
    const w=canvas.width,h=canvas.height,p=34;
    ctx.clearRect(0,0,w,h);
    const minx=Math.min(0,...samples.map(s=>s.x))-1,maxx=Math.max(1,...samples.map(s=>s.x))+1;
    const minz=Math.min(-12,...samples.map(s=>s.z))-2,maxz=12;
    const tx=x=>p+(x-minx)/(maxx-minx)*(w-2*p);
    const tz=z=>h-p-(z-minz)/(maxz-minz)*(h-2*p);
    ctx.lineWidth=1;ctx.strokeStyle='#53695e';ctx.beginPath();
    ctx.moveTo(p,tz(0));ctx.lineTo(w-p,tz(0));ctx.stroke();
    ctx.strokeStyle='#d4f2b5';ctx.lineWidth=2;ctx.beginPath();
    samples.forEach((s,i)=>{if(i===0)ctx.moveTo(tx(s.x),tz(s.z));else ctx.lineTo(tx(s.x),tz(s.z))});ctx.stroke();
    const last=samples[samples.length-1];
    ctx.fillStyle='#d4f2b5';ctx.beginPath();ctx.arc(tx(last.x),tz(last.z),5,0,Math.PI*2);ctx.fill();
    ctx.fillStyle='#a4bbac';ctx.font='12px Arial';ctx.fillText('Original VDrift rigid-body free motion (no collisions)',p,20);
  }
  el('body-status').textContent='Original RIGIDBODY C++ simulation ready';
 };
 for(const input of [seconds,force,torque])input.addEventListener('input',()=>{
  try{draw()}catch(e){el('body-status').textContent=e.message;console.error(e)}
 });
 draw();
};
