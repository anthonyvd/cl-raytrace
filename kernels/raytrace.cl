constant float3 camera_position = { 0, 5, 0 };
constant float3 camera_look_at = { 0, 0, 30 };
constant float3 camera_up = { 0, 1, 0 };

constant float distance_to_film = 10;
constant float2 film_size = { 10, 10 };
constant float2 film_resolution = { 480, 480 };

constant float3 sphere_origin = { 0, 5, 30 };
constant float radius = 5;

constant float3 floor_origin = { 0, -10, 0 };
constant float3 floor_normal = { 0, 1, 0 };

constant float3 light_position = { 100, 100, 30 };


int2 get_coords_from_id(size_t id) {
  return (int2)(id % 480, id / 480);
}

// TODO: support rays originating from inside the sphere
bool ray_sphere_intersection(float3 sphere_origin, float sphere_radius, float3 ray_origin, float3 ray_direction, float3* intersection) {
  float3 v = ray_direction;
  float3 u = sphere_origin - ray_origin;
  float3 puv = (dot(v, u) / length(v)) * v;
  float3 projected = ray_origin + puv;
  float projected_distance_to_sphere_center = length(sphere_origin - projected);

  if(projected_distance_to_sphere_center > radius) return false;
  if(projected_distance_to_sphere_center == radius) {
    *intersection = projected;
    return true;
  }

  float other = length(projected - sphere_origin);
  float dist_to_intersection = length(projected - ray_origin) - sqrt(sphere_radius * sphere_radius + other * other);
  *intersection = ray_origin + ray_direction * dist_to_intersection;
  return true;
}

bool ray_plane_intersection(float3 plane_origin, float3 plane_normal, float3 ray_origin, float3 ray_direction, float3* intersection) {
  float dot_ln = dot(ray_direction, plane_normal);
  if(dot_ln == 0.) return false;
  float distance = dot(plane_origin - ray_origin, plane_normal) / dot_ln;
  if(distance <= 0.f) return false;
  *intersection = ray_origin + ray_direction * distance;
  return true;
}

void kernel render(write_only image2d_t output) {
  float2 film_pixel_size = { film_size.x / film_resolution.x, film_size.y / film_resolution.y };

  int2 coords = get_coords_from_id(get_global_id(0));
  uint x = coords.x;
  uint y = coords.y;

  float3 front = normalize(camera_look_at - camera_position);
  float3 left = normalize(cross(front, camera_up));
  float3 up = -normalize(cross(front, left));

  float3 film_origin = front * distance_to_film + camera_position;

  float3 film_intersection = film_origin +
                             left * film_size.x / 2 +
                             up * film_size.y / 2 -
                             x * film_pixel_size.x * left -
                             y * film_pixel_size.y * up;
  float3 ray_direction = normalize(film_intersection - camera_position);

  float3 intersection;
  if(ray_sphere_intersection(sphere_origin, radius, camera_position, ray_direction, &intersection))
    write_imagef(output, coords, (float4)(0, 1, (length(intersection - camera_position) - 8) / 20, 1));
  else if(ray_plane_intersection(floor_origin, floor_normal, camera_position, ray_direction, &intersection))
    write_imagef(output, coords, (float4)(1, 0, length(intersection - camera_position) / 60, 1));
  else
    write_imagef(output, coords, (float4)(1, 1, 1, 1));
}
