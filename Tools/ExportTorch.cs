using UnityEngine;
using System;
using UnityEditor;
using System.IO;
using System.Linq;
using System.Text;
using System.Globalization;
public static class ExportTorch {
 public static void WriteMesh() {
  var path="Assets/KayKit_DungeonRemastered_1.1_FREE/Assets/fbx/torch_lit.fbx";
  var mesh=AssetDatabase.LoadAllAssetsAtPath(path).OfType<Mesh>().First();
  var b=new StringBuilder("o torch_lit\n");
  foreach(var v in mesh.vertices) b.AppendLine(FormattableString.Invariant($"v {-v.x} {v.y} {v.z}"));
  foreach(var v in mesh.normals) b.AppendLine(FormattableString.Invariant($"vn {-v.x} {v.y} {v.z}"));
  foreach(var v in mesh.uv) b.AppendLine(FormattableString.Invariant($"vt {v.x} {v.y}"));
  var ts=mesh.triangles;
  for(int i=0;i<ts.Length;i+=3) { int a=ts[i]+1,c=ts[i+1]+1,d=ts[i+2]+1; b.AppendLine($"f {a}/{a}/{a} {d}/{d}/{d} {c}/{c}/{c}"); }
  File.WriteAllText("../unity-export/torch_lit.obj",b.ToString());

 }
}
