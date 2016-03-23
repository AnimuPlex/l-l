/**
* Project: VSXu: Realtime modular visual programming engine.
*
* This file is part of Vovoid VSXu.
*
* @author Jonatan Wallmander, Robert Wenzel, Vovoid Media Technologies AB Copyright (C) 2003-2013
* @see The GNU Lesser General Public License (LGPL)
*
* VSXu Engine is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU Lesser General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/



class module_mesh_compute_tangents : public vsx_module
{
public:
  // in
  vsx_module_param_mesh* mesh_in;

  // out

  vsx_module_param_quaternion_array* tangents;
  // internal
  vsx_quaternion_array<> i_tangents;
  vsx_ma_vector< vsx_quaternion<> > data;

  bool init()
  {
    return true;
  }

  void on_delete()
  {
  }

  void module_info(vsx_module_specification* info)
  {
    info->identifier =
      "mesh;modifiers;helpers;mesh_compute_tangents";

    info->description =
      "Computes tangent space for the mesh";

    info->in_param_spec =
      "mesh_in:mesh";

    info->out_param_spec =
      "tangents:quaternion_array";

    info->component_class =
      "mesh";
  }

  void declare_params(vsx_module_param_list& in_parameters, vsx_module_param_list& out_parameters)
  {
    mesh_in = (vsx_module_param_mesh*)in_parameters.create(VSX_MODULE_PARAM_ID_MESH,"mesh_in");
    loading_done = true;
    tangents = (vsx_module_param_quaternion_array*)out_parameters.create(VSX_MODULE_PARAM_ID_QUATERNION_ARRAY,"tangents");
    i_tangents.data = &data;
    tangents->set_p(i_tangents);
  }

  unsigned long prev_timestamp;
  vsx_vector3<> v;

  void run()
  {
    // mesh pointer
    vsx_mesh<>** p = mesh_in->get_addr();

    // is connected / valid ?
    if (!p)
      return;

    // only run if timestamp differs
    if (prev_timestamp == (*p)->timestamp)
      return;

    prev_timestamp = (*p)->timestamp;

    // check for already existing data, use that if found.
    if ((*p)->data->vertex_tangents.size())
    {
      i_tangents.data = &(*p)->data->vertex_tangents;
      return;
    }

    // compute our own data
    data.allocate((*p)->data->vertices.size());
    data.reset_used((*p)->data->vertices.size());
    data.memory_clear();
    vsx_quaternion<>* vec_d = data.get_pointer();
    for (unsigned long a = 0; a < (*p)->data->faces.size(); a++)
    {
      long i1 = (*p)->data->faces[a].a;
      long i2 = (*p)->data->faces[a].b;
      long i3 = (*p)->data->faces[a].c;

      const vsx_vector3<>& v1 = (*p)->data->vertices[i1];
      const vsx_vector3<>& v2 = (*p)->data->vertices[i2];
      const vsx_vector3<>& v3 = (*p)->data->vertices[i3];

      const vsx_tex_coord2f& w1 = (*p)->data->vertex_tex_coords[i1];
      const vsx_tex_coord2f& w2 = (*p)->data->vertex_tex_coords[i2];
      const vsx_tex_coord2f& w3 = (*p)->data->vertex_tex_coords[i3];

      float x1 = v2.x - v1.x;
      float x2 = v3.x - v1.x;
      float y1 = v2.y - v1.y;
      float y2 = v3.y - v1.y;
      float z1 = v2.z - v1.z;
      float z2 = v3.z - v1.z;

      float s1 = w2.s - w1.s;
      float s2 = w3.s - w1.s;
      float t1 = w2.t - w1.t;
      float t2 = w3.t - w1.t;

      float r = 1.0f / (s1 * t2 - s2 * t1);
      vsx_quaternion<> sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);

      vec_d[i1] += sdir;
      vec_d[i2] += sdir;
      vec_d[i3] += sdir;
    }
    for (unsigned long a = 0; a < data.size(); a++)
    {
      vsx_vector3<>& n = (*p)->data->vertex_normals[a];
      vsx_quaternion<>& t = vec_d[a];

      // Gram-Schmidt orthogonalize
      //vec_d[a] = (t - n * t.dot_product(&n) );
      vec_d[a] = (t - n * t.dot_product(&n) );
      vec_d[a].normalize();

      // Calculate handedness
      //tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
    }
  }
};
