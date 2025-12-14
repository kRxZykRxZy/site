// Minimal CodeMirror 6 integration using a simple textarea fallback.
(async function(){
  const projectId = window.PROJECT_ID || 1;
  const editorEl = document.getElementById('editor');
  const saveBtn = document.getElementById('save');
  // load a default file
  const path = 'index.html';
  const res = await fetch(`/api/projects/${projectId}/files/raw/${encodeURIComponent(path)}`);
  let content = '';
  if (res.ok) {
    const body = await res.json();
    content = body.content;
  }
  const ta = document.createElement('textarea');
  ta.style.width='100%'; ta.style.height='100%';
  ta.value = content;
  editorEl.innerHTML = '';
  editorEl.appendChild(ta);

  saveBtn.addEventListener('click', async ()=>{
    const form = new FormData();
    form.append('path', path);
    form.append('content', ta.value);
    const r = await fetch(`/api/projects/${projectId}/files/enqueue`, {method:'POST', body: form});
    if (r.ok) alert('Enqueued'); else alert('Error');
  });
})();
