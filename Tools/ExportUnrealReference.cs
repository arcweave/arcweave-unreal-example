using UnityEngine;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine.SceneManagement;
using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using System.Reflection;

public static class ExportUnrealReference
{
    [Serializable] public class MaterialInfo { public string name, asset, texture; public Color color; }
    [Serializable] public class MeshInfo { public string name, asset, mesh; public Vector3 position, scale, meshCenter, meshSize; public Quaternion rotation; public bool skinned, active, collider; public MaterialInfo[] materials; public string parent; }
    [Serializable] public class ScriptInfo { public string type, json; }
    [Serializable] public class ObjectInfo { public string name, path; public bool active; public Vector3 position, scale; public Quaternion rotation; public ScriptInfo[] scripts; }
    [Serializable] public class LightInfo { public string name, type; public Vector3 position; public Quaternion rotation; public Color color; public float intensity, range, spotAngle; public bool shadows, active; }
    [Serializable] public class ClipInfo { public string name, asset; public float length; public bool loop; }
    [Serializable] public class Reference { public MeshInfo[] meshes; public ObjectInfo[] objects; public LightInfo[] lights; public ClipInfo[] clips; public Color ambient, background; public float fieldOfView; public Vector3 cameraPosition; public Quaternion cameraRotation; }
    static string ObjectPath(Transform t) => t.parent ? ObjectPath(t.parent) + "/" + t.name : t.name;
    static string Output => Path.GetFullPath("../unity-export");
    public static void Run()
    {
        Directory.CreateDirectory(Output);
        ExportTorch.WriteMesh();
        EditorSceneManager.OpenScene("Assets/Scenes/Demo Scene.unity", OpenSceneMode.Single);
        var all = SceneManager.GetActiveScene().GetRootGameObjects().SelectMany(g => g.GetComponentsInChildren<Transform>(true)).ToArray();
        var renderers = all.SelectMany(t => t.GetComponents<Renderer>()).Where(r => r is MeshRenderer || r is SkinnedMeshRenderer).ToArray();
        var meshes = new List<MeshInfo>();
        foreach (var r in renderers)
        {
            var filter = r.GetComponent<MeshFilter>();
            var mesh = r is SkinnedMeshRenderer sk ? sk.sharedMesh : (filter ? filter.sharedMesh : null);
            if (!mesh) continue;
            var mats = r.sharedMaterials.Select(m => new MaterialInfo { name=m ? m.name : "", asset=AssetDatabase.GetAssetPath(m), texture=m ? AssetDatabase.GetAssetPath(m.mainTexture) : "", color=m && m.HasProperty("_BaseColor") ? m.GetColor("_BaseColor") : (m && m.HasProperty("_Color") ? m.color : Color.white) }).ToArray();
            meshes.Add(new MeshInfo { name=r.name, parent=ObjectPath(r.transform), asset=AssetDatabase.GetAssetPath(mesh), mesh=mesh.name, position=r.transform.position, rotation=r.transform.rotation, scale=r.transform.lossyScale, meshCenter=mesh.bounds.center, meshSize=mesh.bounds.size, active=r.gameObject.activeInHierarchy && r.enabled, skinned=r is SkinnedMeshRenderer, collider=r.GetComponent<Collider>() != null, materials=mats });
        }
        var objects = all.Where(t => t.GetComponents<MonoBehaviour>().Length > 0 || t.GetComponent<Camera>() || t.GetComponent<Animator>()).Select(t => new ObjectInfo { name=t.name, path=ObjectPath(t), active=t.gameObject.activeInHierarchy, position=t.position, rotation=t.rotation, scale=t.lossyScale, scripts=t.GetComponents<MonoBehaviour>().Where(s => s).Select(s => new ScriptInfo { type=s.GetType().Name, json=EditorJsonUtility.ToJson(s) }).ToArray() }).ToArray();
        var lights = all.SelectMany(t => t.GetComponents<Light>()).Select(l => new LightInfo {name=l.name,type=l.type.ToString(),position=l.transform.position,rotation=l.transform.rotation,color=l.color,intensity=l.intensity,range=l.range,spotAngle=l.spotAngle,shadows=l.shadows!=LightShadows.None,active=l.isActiveAndEnabled}).ToArray();
        var clips = AssetDatabase.FindAssets("t:AnimationClip").Select(AssetDatabase.GUIDToAssetPath).Distinct().SelectMany(p => AssetDatabase.LoadAllAssetsAtPath(p).OfType<AnimationClip>().Select(c => new ClipInfo {name=c.name,asset=p,length=c.length,loop=c.isLooping})).ToArray();
        var cam = Camera.main;
        var result = new Reference { meshes=meshes.ToArray(), objects=objects, lights=lights, clips=clips, ambient=RenderSettings.ambientLight, background=cam.backgroundColor, fieldOfView=cam.fieldOfView, cameraPosition=cam.transform.position, cameraRotation=cam.transform.rotation };
        File.WriteAllText(Path.Combine(Output,"scene.json"), JsonUtility.ToJson(result,true));
        Debug.Log("REFERENCE_EXPORTED " + meshes.Count + " meshes, " + clips.Length + " clips");
        var rt = new RenderTexture(1280,720,24);
        cam.targetTexture=rt;
        cam.Render();
        RenderTexture.active=rt;
        var tex=new Texture2D(1280,720,TextureFormat.RGB24,false);
        tex.ReadPixels(new Rect(0,0,1280,720),0,0);
        tex.Apply();
        File.WriteAllBytes(Path.Combine(Output,"reference.png"),tex.EncodeToPNG());
        cam.targetTexture=null;
        RenderTexture.active=null;
        UnityEngine.Object.DestroyImmediate(rt);
        UnityEngine.Object.DestroyImmediate(tex);
        EditorApplication.Exit(0);
    }
}
