// Real VDrift tyre curves. All force samples call the compiled C++ CARTIRE methods.
window.initFayawoTires = async function(module,manifest) {
  const chooser=document.getElementById('tire-select');
  const state=document.getElementById('tire-status');
  const canvas=document.getElementById('tire-chart');
  if(!chooser||!state||!canvas)return;
  const loadTire=module.cwrap('fayawo_tire_config_load','number',['string']);
  const names=manifest.tires||[];
  chooser.replaceChildren();
  for(const name of names) {
    const option=document.createElement('option');
    option.value=name;option.textContent=name;chooser.append(option);
  }
  if(!names.length){state.textContent='No original tyre configurations';return}
  chooser.disabled=false;
  function draw(){
    const ctx=canvas.getContext('2d');
    if(!ctx)return;
    const w=canvas.width,h=canvas.height,p=36;
    ctx.clearRect(0,0,w,h);
    const longitudinal=[],lateral=[];
    for(let i=0;i<=160;i++){
      const t=i/160;
      longitudinal.push(module._fayawo_tire_longitudinal_force((t-.5)*.6,4000));
      lateral.push(module._fayawo_tire_lateral_force((t-.5)*24,4000));
    }
    const peak=Math.max(100,...longitudinal.map(x=>Math.abs(x)),...lateral.map(x=>Math.abs(x)))*1.08;
    ctx.strokeStyle='#50665a';ctx.lineWidth=1;ctx.beginPath();ctx.moveTo(p,h/2);ctx.lineTo(w-p,h/2);ctx.stroke();
    const curves=[{values:longitudinal,color:'#d4f2b5',label:'Longitudinal'},{values:lateral,color:'#a1b5ee',label:'Lateral'}];
    ctx.font='12px Arial';ctx.textAlign='left';
    curves.forEach((curve,k)=>{
      ctx.strokeStyle=curve.color;ctx.lineWidth=2;ctx.beginPath();
      curve.values.forEach((force,i)=>{
        const x=p+(w-2*p)*i/160,y=h/2-force/peak*(h/2-p);
        if(i===0)ctx.moveTo(x,y);else ctx.lineTo(x,y);
      });
      ctx.stroke();ctx.fillStyle=curve.color;ctx.fillText(curve.label,p+k*135,20);
    });
    ctx.fillStyle='#afc4b5';ctx.fillText('-0.30 / -12 deg',p,h-10);
    ctx.fillText('0',w/2,h-10);ctx.textAlign='right';ctx.fillText('+0.30 / +12 deg',w-p,h-10);
  }
  async function load(name){
    const response=await fetch('./assets/tires/'+encodeURIComponent(name));
    if(!response.ok)throw Error('Tyre asset missing: '+name);
    const source=await response.text();
    if(loadTire(source)!==1)throw Error('Original .tire parser rejected '+name);
    draw();
    state.textContent='Original '+name+' coefficients loaded by VDrift C++';
  }
  chooser.addEventListener('change',()=>load(chooser.value).catch(e=>{state.textContent=e.message;console.error(e)}));
  chooser.value=names.includes('asphalt.tire')?'asphalt.tire':names[0];
  await load(chooser.value);
};
