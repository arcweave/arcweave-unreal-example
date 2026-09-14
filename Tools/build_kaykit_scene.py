"""Run with UnrealEditor-Cmd -run=pythonscript -script=<this file>."""
import json
from pathlib import Path
import traceback
import unreal

ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'SourceAssets/KayKit'
DATA=json.loads((SOURCE/'scene.json').read_text(encoding='utf-8-sig'))
DEST='/Game/KayKit'
TOOLS=unreal.AssetToolsHelpers.get_asset_tools()
LIB=unreal.EditorAssetLibrary
LOG=ROOT/'ArcweaveDemo/Saved/Validation'
LOG.mkdir(parents=True,exist_ok=True)
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')

def position(p): return unreal.Vector(p['z']*100,p['x']*100,p['y']*100)
def rotation(q): return unreal.Quat(q['z'],q['x'],q['y'],q['w']).rotator()
def scale(s): return unreal.Vector(s['z'],s['x'],s['y'])

def import_file(file,folder,name,options=None,force=False):
    existing=f'{folder}/{name}'
    if LIB.does_asset_exist(existing) and not force: return [existing]
    task=unreal.AssetImportTask()
    task.filename=str(file)
    task.destination_path=folder
    task.destination_name=name
    task.automated=True
    task.replace_existing=True
    task.save=True
    if options: task.options=options
    TOOLS.import_asset_tasks([task])
    paths=list(task.imported_object_paths)
    if not paths: raise RuntimeError(f'Import failed: {file}')
    return paths

materials={}
for texture in DATA['textures']:
    name=texture['name']
    texture_path=import_file(SOURCE/texture['file'],DEST+'/Textures',name)[0]
    mat_path=DEST+'/Materials/M_'+name
    mat=LIB.load_asset(mat_path) if LIB.does_asset_exist(mat_path) else TOOLS.create_asset('M_'+name,DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    if name!='dungeon_texture': mat.set_editor_property('used_with_skeletal_mesh',True)
    # Rebuild graph to make this script repeatable.
    unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)
    sample=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionTextureSample,-300,0)
    sample.texture=LIB.load_asset(texture_path)
    unreal.MaterialEditingLibrary.connect_material_property(sample,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
    rough=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant,-300,250)
    rough.r=1
    unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    LIB.save_loaded_asset(mat)
    materials[name]=mat

for portrait in (SOURCE/'UI').glob('*.png'):
    tex=LIB.load_asset(import_file(portrait,DEST+'/UI',portrait.stem)[0])
    tex.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    tex.set_editor_property('never_stream',True)
    LIB.save_loaded_asset(tex)

story=json.loads((ROOT/'ArcweaveDemo/Content/ArcweaveExport/project_settings.json').read_text(encoding='utf-8'))
sounds={}
for element_id,element in story['elements'].items():
    for sound in element.get('assets',{}).get('audio',[]):
        resource=SOURCE/'UI'/story['assets'][sound['asset']]['name']
        sounds[element_id]=LIB.load_asset(import_file(resource,DEST+'/Audio',resource.stem)[0])

models={}
report={}
for entry in DATA['assets']:
    name=entry['name']
    folder=DEST+('/Characters/'+name if entry['skeletal'] else '/Environment')
    options=unreal.FbxImportUI()
    options.automated_import_should_detect_type=False
    options.import_as_skeletal=entry['skeletal']
    options.mesh_type_to_import=unreal.FBXImportType.FBXIT_SKELETAL_MESH if entry['skeletal'] else unreal.FBXImportType.FBXIT_STATIC_MESH
    options.import_materials=False
    options.import_textures=False
    options.import_animations=entry['skeletal']
    options.create_physics_asset=False
    details=options.skeletal_mesh_import_data if entry['skeletal'] else options.static_mesh_import_data
    details.convert_scene=True
    details.convert_scene_unit=True
    details.force_front_x_axis=True
    details.normal_import_method=unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS
    if not entry['skeletal']:
        details.combine_meshes=True
        details.generate_lightmap_u_vs=False
        details.auto_generate_collision=True
    source_file=SOURCE/entry['file']
    is_obj=source_file.suffix.lower()=='.obj'
    if is_obj:
        # OBJ has no unit/up-axis metadata. Interchange treats it as centimetres,
        # Z up, and reverses Y. Encode Unity's mesh coordinates explicitly.
        converted=LOG/(name+'.obj')
        lines=[]
        for line in source_file.read_text().splitlines():
            parts=line.split()
            if parts and parts[0] in ('v','vn'):
                x,y,z=map(float,parts[1:4]); factor=100 if parts[0]=='v' else 1
                line=f'{parts[0]} {z*factor} {x*factor} {y*factor}'
            if not line.startswith('mtllib '): lines.append(line)
        converted.write_text('\n'.join(lines)+'\n')
        source_file=converted
    paths=import_file(source_file,folder,name,options,force=is_obj)
    candidates=[LIB.load_asset(p) for p in paths]
    wanted=unreal.SkeletalMesh if entry['skeletal'] else unreal.StaticMesh
    mesh=next((m for m in candidates if isinstance(m,wanted)),None)
    if mesh is None: raise RuntimeError(f'No mesh imported for {name}: {paths}')
    models[name]=mesh
    if not entry['skeletal']:
        body=mesh.get_editor_property('body_setup')
        if body: body.set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        LIB.save_loaded_asset(mesh)
    report[name]={'mesh':mesh.get_path_name(),'assets':paths}
    unreal.log(f'KAYKIT_IMPORTED {name}')
    (LOG/'kaykit_imports.json').write_text(json.dumps(report,indent=2),encoding='utf-8')

LEVEL=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ACTORS=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world=unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
world.get_world_settings().set_editor_property('default_game_mode',unreal.KayKitGameMode.static_class())
world.get_world_settings().set_editor_property('force_no_precomputed_lighting',True)

placed=set()
for index,entry in enumerate(DATA['meshes']):
    key=(entry['asset_name'],tuple(round(entry['position'][a],4) for a in 'xyz'))
    if key in placed: continue # The Unity scene contains overlapping duplicate floor tiles.
    placed.add(key)
    rot=rotation(entry['rotation'])
    size=scale(entry['scale'])
    actor=ACTORS.spawn_actor_from_class(unreal.StaticMeshActor,position(entry['position']),rot)
    actor.set_actor_label(entry['parent'].replace('/','_'))
    actor.set_folder_path('Dungeon')
    actor.static_mesh_component.set_static_mesh(models[entry['asset_name']])
    actor.set_actor_scale3d(size)
    for slot,mat_name in enumerate(entry['materials']): actor.static_mesh_component.set_material(slot,materials[mat_name])
    actor.static_mesh_component.set_mobility(unreal.ComponentMobility.STATIC)
    if entry['asset_name']=='torch_lit': actor.static_mesh_component.set_cast_shadow(False)
    if entry['name'].startswith('bottle_') and entry['parent'].startswith('Characters/Healer'):
        actor.tags=['ArcweavePotion']

for entry in DATA['characters']:
    pos=position(entry['position'])+unreal.Vector(0,0,90)
    actor=ACTORS.spawn_actor_from_class(unreal.KayKitCharacter,pos,rotation(entry['rotation']))
    actor.set_actor_label(entry['name'])
    actor.set_folder_path('Characters')
    actor.set_editor_property('character_name',entry['name'])
    actor.set_editor_property('dialogue_board',entry['board'])
    actor.set_editor_property('health_variable',entry['health'])
    actor.set_editor_property('dialogue_sounds',sounds)
    actor.set_editor_property('health_color',unreal.LinearColor(.83,.26,.25,1) if entry['name']=='Healer' else unreal.LinearColor(.23,.72,.38,1))
    actor.mesh.set_skeletal_mesh_asset(models[entry['asset_name']])
    # Match the animated Unity silhouettes at the shared camera distance.
    # Unreal combines the FBX's rigid attachments into the skeletal mesh.
    actor.mesh.set_relative_scale3d(unreal.Vector(.8,.8,.8))
    for slot in range(actor.mesh.get_num_materials()): actor.mesh.set_material(slot,materials[entry['material']])
    animations=[LIB.load_asset(p) for p in LIB.list_assets(DEST+'/Characters/'+entry['asset_name'])]
    animations=[a for a in animations if isinstance(a,unreal.AnimSequence)]
    def animation(suffix):
        return next((a for a in animations if a.get_name()==entry['asset_name']+'_Anim_'+suffix),None)
    for property_name,suffix in [('idle_animation','Idle'),('walk_animation','Walking_B'),('injured_animation','Lie_Idle'),('attack_animation','1H_Melee_Attack_Slice_Diagonal')]:
        clip=animation(suffix)
        if clip: actor.set_editor_property(property_name,clip)
    if not entry['board']: actor.set_editor_property('auto_possess_player',unreal.AutoReceiveInput.PLAYER0)

sky=ACTORS.spawn_actor_from_class(unreal.SkyLight,unreal.Vector(0,0,500))
sky.set_actor_label('Soft ambient light')
sky.light_component.set_editor_property('source_type',unreal.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP)
sky.light_component.set_editor_property('cubemap',unreal.load_object(None,'/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap'))
sky.light_component.set_intensity(3.0)
sky.light_component.set_mobility(unreal.ComponentMobility.MOVABLE)

for entry in DATA['lights']:
    if not entry['active'] or entry['type']!='Point': continue
    light=ACTORS.spawn_actor_from_class(unreal.PointLight,position(entry['position']))
    light.set_actor_label('Torch light')
    component=light.point_light_component
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    component.set_editor_property('use_inverse_squared_falloff',False)
    component.set_intensity(entry['intensity'])
    component.set_attenuation_radius(entry['range']*100)
    color=entry['color']
    component.set_light_color(unreal.LinearColor(color['r'],color['g'],color['b'],1))
    component.set_cast_shadows(entry['shadows'] and entry['range']>2)

pp=ACTORS.spawn_actor_from_class(unreal.PostProcessVolume,unreal.Vector())
pp.set_editor_property('unbound',True)
settings=pp.get_editor_property('settings')
settings.set_editor_property('override_auto_exposure_method',True)
settings.set_editor_property('auto_exposure_method',unreal.AutoExposureMethod.AEM_MANUAL)
settings.set_editor_property('override_auto_exposure_bias',True)
settings.set_editor_property('auto_exposure_bias',0)
settings.set_editor_property('override_auto_exposure_apply_physical_camera_exposure',True)
settings.set_editor_property('auto_exposure_apply_physical_camera_exposure',False)
settings.set_editor_property('override_motion_blur_amount',True)
settings.set_editor_property('motion_blur_amount',0)
pp.set_editor_property('settings',settings)

sky_mat_path=DEST+'/Materials/M_Background'
sky_mat=LIB.load_asset(sky_mat_path) if LIB.does_asset_exist(sky_mat_path) else TOOLS.create_asset('M_Background',DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
unreal.MaterialEditingLibrary.delete_all_material_expressions(sky_mat)
sky_mat.set_editor_property('two_sided',True)
sky_mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
color=unreal.MaterialEditingLibrary.create_material_expression(sky_mat,unreal.MaterialExpressionVectorParameter)
color.set_editor_property('parameter_name','SkyColor')
color.set_editor_property('default_value',unreal.LinearColor(.24,.3,.33,1))
unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
unreal.MaterialEditingLibrary.recompile_material(sky_mat)
background=ACTORS.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector())
background.set_actor_label('Background')
background.tags=['ArcweaveBackground']
background.static_mesh_component.set_static_mesh(unreal.load_object(None,'/Engine/BasicShapes/Sphere.Sphere'))
background.static_mesh_component.set_material(0,sky_mat)
background.static_mesh_component.set_cast_shadow(False)
background.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
background.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
background.set_actor_enable_collision(False)
background.set_actor_scale3d(unreal.Vector(100,100,100))

rain_path=DEST+'/Materials/M_Rain'
rain=LIB.load_asset(rain_path) if LIB.does_asset_exist(rain_path) else TOOLS.create_asset('M_Rain',DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
rain.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
rain.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
rain.set_editor_property('used_with_instanced_static_meshes',True)
unreal.MaterialEditingLibrary.delete_all_material_expressions(rain)
color=unreal.MaterialEditingLibrary.create_material_expression(rain,unreal.MaterialExpressionConstant3Vector)
color.set_editor_property('constant',unreal.LinearColor(.65,.75,.8,1))
unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
opacity=unreal.MaterialEditingLibrary.create_material_expression(rain,unreal.MaterialExpressionConstant)
opacity.r=.25
unreal.MaterialEditingLibrary.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
unreal.MaterialEditingLibrary.recompile_material(rain)

if not unreal.EditorLoadingAndSavingUtils.save_map(world,DEST+'/Maps/ArcweaveDemo'):
    raise RuntimeError('Unable to save the replacement map')
LIB.save_directory(DEST,only_if_is_dirty=True,recursive=True)
unreal.log('KAYKIT_SCENE_COMPLETE')
