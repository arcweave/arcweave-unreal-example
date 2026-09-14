# Shared example assets

Copied from Arcweave's Unity example, commit `ff909af`.

- Models and texture atlases: KayKit Adventurers 1.0 FREE and KayKit Dungeon
  Remastered 1.1 FREE by Kay Lousberg. Both packs' original CC0 license notices
  are included in this directory.
- `UI`: the portraits and audio used by the Arcweave Unity example.
- `scene.json`: exported scene placement, materials, lights, character roles and
  source paths. Unity metres are converted to Unreal centimetres by the builder.
- `Models/torch_lit.obj`: the Unity-imported mesh exported with its node transform
  left intact. This avoids applying Blender's FBX root transform twice.

See `../../Tools/README.md` for the import and rebuild workflow.
