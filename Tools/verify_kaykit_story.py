"""Run through Unreal's Python console; starts PIE if necessary and resets afterwards."""
import json
from pathlib import Path
import time
import traceback
import unreal

level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
if not level.is_in_play_in_editor(): level.editor_request_begin_play()
report={'checks':[],'passed':False}
report_path=Path(unreal.Paths.project_saved_dir())/'Validation/kaykit_story_verification.json'
phase=0
next_check=time.monotonic()+2

def check(condition,message):
    if not condition: raise AssertionError(message)
    report['checks'].append(message)

def choice(text):
    choices=pc.get_editor_property('choices')
    index=next((i for i,c in enumerate(choices) if text.lower() in c.label.lower()),None)
    check(index is not None,'Choice available: '+text)
    pc.choose(index)

def value(name): return pc.variable_value(name)
def dialogue(): return pc.get_editor_property('dialogue_text')
def animation(actor):
    return actor.get_editor_property('playing_animation').get_name()

def verify_tick(dt):
    global phase,next_check,pc,arc,npcs,world,verification_handle
    if time.monotonic()<next_check: return
    next_check=time.monotonic()+.25
    try:
        if phase==0:
            world=editor.get_game_world()
            pc=unreal.GameplayStatics.get_player_controller(world,0)
            arc=unreal.get_engine_subsystem(unreal.ArcweaveSubsystem)
            npcs={a.character_name:a for a in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.KayKitCharacter)}
            check(len(npcs)==4,'Player and all three Unity NPCs are present')
            pc.end_dialogue(); arc.fetch_data('','')
        elif phase==1:
            check(pc.get_editor_property('bStoryReady'),'Bundled story loads without API credentials')
            check(animation(npcs['Wanda Elandor']).endswith('Lie_Idle'),'Wounded Wanda uses the lying animation')
            pc.start_dialogue(npcs['Wanda Elandor']); choice('Examine')
            check(not any('Give her' in c.label for c in pc.choices),'Potion choice is unavailable before collecting a potion')
            pc.end_dialogue()
            pc.start_dialogue(npcs['Healer']); choice('Ask for help'); pc.choose(0)
            check(value('have_potion')=='true','Healer grants a potion')
            check(pc.get_editor_property('bInDialogue') and 'healing potion' in dialogue(),'Healer final line remains readable')
            check(len(npcs['Healer'].dialogue_sounds)>0,'Story sound assets are bound')
            pc.choose(0)
            check(not pc.get_editor_property('bInDialogue'),'Final choice returns to gameplay')
            pc.start_dialogue(npcs['Wanda Elandor']); choice('Examine'); choice('Give her')
            check(value('wanda_health')=='70' and value('have_potion')=='false','Potion heals Wanda from 20 to 70 and is consumed')
        elif phase==2:
            check(animation(npcs['Wanda Elandor']).endswith('_Idle') and not animation(npcs['Wanda Elandor']).endswith('Lie_Idle'),'Healed Wanda returns to standing idle')
            pc.end_dialogue(); pc.start_dialogue(npcs['Wanda Elandor'])
            check('saving my life' in dialogue(),'Revisited Wanda uses the healed branch')
            pc.end_dialogue(); pc.start_dialogue(npcs['Fighter']); choice('Train me')
            check(value('gold')=='0' and value('sword_locked')=='false','Fighter training spends 50 gold and unlocks the sword')
            check(pc.get_editor_property('bCanAttack'),'Attack component enables combat')
            check(pc.get_editor_property('bInDialogue') and 'attack enemies' in dialogue(),'Fighter final instruction remains readable')
            pc.choose(0); npcs['Player'].attack(1)
        elif phase==3:
            check('1H_Melee_Attack_Slice_Diagonal' in animation(npcs['Player']),'Unlocked attack plays the Unity attack animation')
            arc.fetch_data('','')
            variables=arc.get_arcweave_project_data().current_vars
            gold_id=next(k for k,v in variables.items() if v.name=='gold')
            arc.set_variable(gold_id,'0')
            pc.start_dialogue(npcs['Fighter'])
            check(not any('Train me' in c.label for c in pc.choices),'Training is unavailable without enough gold')
            choice('Not enough gold')
            check(value('sword_locked')=='true' and value('gold')=='0','Insufficient gold does not unlock the sword or spend gold')
            check('afford' in dialogue(),'Insufficient funds follows the correct story branch')
            pc.end_dialogue(); pc.toggle_menu()
            check(pc.get_editor_property('bMenuOpen'),'Import menu opens')
            pc.toggle_menu(); check(not pc.get_editor_property('bMenuOpen'),'Import menu closes')
            arc.fetch_data('','')
            check(value('gold')=='50' and value('sword_locked')=='true','Reloading the example resets story state')
            report['passed']=True
            report_path.parent.mkdir(parents=True,exist_ok=True)
            report_path.write_text(json.dumps(report,indent=2))
            unreal.unregister_slate_post_tick_callback(verification_handle)
            unreal.log('KAYKIT_STORY_VERIFICATION_PASSED')
            return
        phase+=1
    except Exception:
        report['error']=traceback.format_exc()
        report_path.parent.mkdir(parents=True,exist_ok=True)
        report_path.write_text(json.dumps(report,indent=2))
        unreal.unregister_slate_post_tick_callback(verification_handle)
        unreal.log_error(report['error'])

verification_handle=unreal.register_slate_post_tick_callback(verify_tick)
