import json
from pathlib import Path

discovery={'version':1,'device':{'id':'fixture-device','label':'Local test PC','repos':['demo/briareus'],'permission':'manage','expiresAt':1990000000000},'transcribe':False}
session={'id':'session-1','repo':'demo/briareus','title':'Improve project search','status':'running','provider':'fixture','model':'test-model'}
pull={'number':42,'title':'Preserve the selected conversation','body':'Fix selection when refreshing the project list.\n\n## Validation\nUnicode and keyboard checks passed in this fixture.','headRef':'fix/selection','headSha':'abc123','baseSha':'def456','baseRef':'main','labels':[{'name':'enhancement'}],'author':{'login':'demo'},'mergeable':True,'checks':[{'name':'unit tests','status':'completed','conclusion':'success'}]}
read=['projects','sessions','session','pulls','pull','pull_files','findings','actions','branches','runtimes']
write=['start_session','message','rename','cancel','close','reopen','delete','review_loop','complete_findings','finding_decision','merge_pull','serve_pull','review','solve_conflicts','fix_checks','implement_feedback','custom_feedback','test_sheet','qa','pr_body_summary','delete_self_comments','drop_message']
fixtures={
 'Discovery':discovery,
 'ReadOnly':{**discovery,'device':{**discovery['device'],'permission':'read'}},
 'Operations':{'operations':[{'name':n,'readOnly':n in read} for n in read+write]},
 'Projects':{'projects':[{'repo':'demo/briareus','label':'Briareus · demo'},{'repo':'demo/mobile','label':'Mobile · demo'}]},
 'Sessions':{'sessions':[session,{**session,'id':'session-2','title':'Review the mobile API','status':'idle'}]},
 'Session':{'session':session,'events':[{'seq':1,'kind':'setup','text':'MUST NOT RENDER'},{'seq':2,'kind':'user','t':'2026-09-29T09:00:00Z','text':'Keep the selected conversation while I refresh.'},{'seq':3,'kind':'assistant','t':'2026-09-29T09:00:01Z','text':'I will preserve the selection by its stable session ID.\n\nUnicode check: España — 日本語 🚀'},{'seq':4,'kind':'tool','t':'2026-09-29T09:00:02Z','summary':'Read ProjectsView.swift'},{'seq':5,'kind':'question','t':'2026-09-29T09:00:03Z','question':'Should archived conversations remain visible?','options':['Yes','No']}]},
 'Pulls':{'pulls':[pull],'issues':[{'number':7,'title':'Keep conversation selection','body':'Refresh should preserve focus.','labels':[{'name':'bug'}],'author':{'login':'demo'}}]},
 'Pull':{'pr':pull},
 'Files':{'pr':pull,'files':[{'filename':'src/selection.asm','status':'modified','additions':1,'deletions':1,'patch':'@@ -1 +1 @@\n- mov eax,0\n+ mov eax,[selectedRow]'}],'nextPage':None},
 'Empty':{'ok':True}
}
lines=["testLogPath du 'build/test-requests.jsonl',0",'testLogNewline db 10']
for name,value in fixtures.items():
    # FASM source contains ASCII only, preserving unicode through JSON escapes.
    data=json.dumps(value,ensure_ascii=True,separators=(',',':'))
    lines.append('fixture'+name+' dw '+','.join(str(ord(ch)) for ch in data)+',0')
Path('tests/fixtures.inc').write_text('\n'.join(lines)+'\n')
