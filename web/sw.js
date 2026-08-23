const CACHE='pi-guardian-v2';
self.addEventListener('install',e=>e.waitUntil(caches.open(CACHE).then(c=>c.addAll(['/','/manifest.json','/icon.svg']))));
self.addEventListener('activate',e=>e.waitUntil(self.clients.claim()));
self.addEventListener('fetch',e=>{if(e.request.method==='GET')e.respondWith(fetch(e.request).catch(()=>caches.match(e.request)))});
self.addEventListener('push',e=>e.waitUntil((async()=>{let d={level:'WARNING',title:'Pi Guardian',body:'Important infrastructure event'};try{const r=await fetch('/api/notifications/latest',{credentials:'include',cache:'no-store'});if(r.ok)d=Object.assign(d,await r.json())}catch(_){}const icon='/icon.svg';await self.registration.showNotification(d.title||'Pi Guardian',{body:d.body||'Important infrastructure event',icon,badge:icon,tag:'pi-guardian-'+(d.level||'event'),renotify:true,data:{url:'/'}})})()));
self.addEventListener('notificationclick',e=>{e.notification.close();e.waitUntil(clients.matchAll({type:'window',includeUncontrolled:true}).then(cs=>{for(const c of cs){if('focus'in c)return c.focus()}return clients.openWindow(e.notification.data?.url||'/')}))});
