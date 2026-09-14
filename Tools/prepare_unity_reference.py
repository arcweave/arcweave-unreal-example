"""Copy the Unity demo's used source assets and normalize its exported scene manifest."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil

REPO = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--unity-project', type=Path, default=REPO.parent / 'arcweave-unity-example')
parser.add_argument('--export', type=Path, default=REPO.parent / 'unity-export/scene.json')
args = parser.parse_args()
UNITY = args.unity_project.resolve()
EXPORT = args.export.resolve()
OUT = REPO / 'SourceAssets/KayKit'
OUT.mkdir(parents=True, exist_ok=True)
scene = json.loads(EXPORT.read_text(encoding='utf-8'))
assets = {}
textures = {}

def asset(source, skeletal=False):
    if source not in assets:
        path = Path(source)
        name = re.sub(r'[^A-Za-z0-9_]', '_', path.stem)
        if any(a['name'] == name for a in assets.values()):
            name += '_' + hashlib.md5(source.encode()).hexdigest()[:6]
        source_file=UNITY/source
        if name=='torch_lit' and not skeletal:
            # Export this mesh from Unity to retain its unbaked node transform.
            source_file=EXPORT.parent/'torch_lit.obj'
        destination = OUT / 'Models' / (name + source_file.suffix)
        destination.parent.mkdir(exist_ok=True)
        shutil.copy2(source_file, destination)
        assets[source] = {'source': source, 'name': name, 'file': str(destination.relative_to(OUT)).replace('\\','/'), 'skeletal': skeletal}
    return assets[source]['name']

def material(entry):
    source=entry['texture']
    if not source:
        return 'dungeon_texture'
    name=Path(source).stem
    if name not in textures:
        destination=OUT/'Textures'/Path(source).name
        destination.parent.mkdir(exist_ok=True)
        shutil.copy2(UNITY/source,destination)
        textures[name]={'name':name,'file':str(destination.relative_to(OUT)).replace('\\','/')}
    return name

meshes=[]
for entry in scene['meshes']:
    if not entry['active'] or not entry['asset'] or entry['skinned']:
        continue
    if entry['parent'].startswith('Characters/') and not entry['name'].startswith('bottle_'):
        continue
    copy=dict(entry)
    copy['asset_name']=asset(entry['asset'])
    copy['materials']=[material(m) for m in entry['materials']]
    meshes.append(copy)

characters=[]
for obj in scene['objects']:
    if not obj['path'].startswith('Characters/') or obj['path'].count('/') != 1:
        continue
    skinned=next(m for m in scene['meshes'] if m['skinned'] and m['parent'].startswith(obj['path']+'/'))
    scripts={s['type']:json.loads(s['json']).get('MonoBehaviour',{}) for s in obj['scripts']}
    characters.append({'name':obj['name'],'position':obj['position'],'rotation':obj['rotation'],'scale':obj['scale'],
                       'asset_name':asset(skinned['asset'],True),'material':material(skinned['materials'][0]),
                       'board':scripts.get('DialogueTrigger',{}).get('specificBoardName',''),
                       'health':scripts.get('ArcweaveHealthUI',{}).get('healthVariableName','')})

manifest={'source_commit':'ff909af','meshes':meshes,'characters':characters,'lights':scene['lights'],
          'assets':list(assets.values()),'textures':list(textures.values()),'camera':{'distance':800,'height':200,'vertical_fov':60},
          'ambient':scene['ambient']}
(OUT/'scene.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
for pack in ['KayKit_Adventurers_1.0_FREE','KayKit_DungeonRemastered_1.1_FREE']:
    shutil.copy2(UNITY/'Assets'/pack/'License.txt',OUT/(pack+'_License.txt'))
story=json.loads((UNITY/'Assets/Arcweave/project.json').read_text(encoding='utf-8'))
(REPO/'ArcweaveDemo/Content/ArcweaveExport/project_settings.json').write_text(json.dumps(story,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
for resource in (UNITY/'Assets/Resources').iterdir():
    if resource.suffix.lower() in ('.png','.mp3','.wav'):
        destination=OUT/'UI'/resource.name
        destination.parent.mkdir(exist_ok=True)
        shutil.copy2(resource,destination)
print(json.dumps({'placed_meshes':len(meshes),'characters':len(characters),'unique_models':len(assets),'textures':len(textures)}))
