struct sphere_s {
  float3 origin;
  float radius;
};

struct ray_s {
  float3 origin;
  float3 direction;
};

struct plane_s {
  float3 origin;
  float3 normal;
};

struct pinhole_camera_s {
  float3 position;
  float3 look_at;
  float3 up;
};

constant struct pinhole_camera_s camera = {
  { 0, 5, 0 },
  { 0, 0, 30 },
  { 0, 1, 0 },
};

constant float distance_to_film = 10;
constant float2 film_size = { 10, 10 };
constant float2 film_resolution = { 480, 480 };

constant struct sphere_s sphere = {
  { 0, 5, 30 },
  5,
};
constant float3 sphere_color = { 1, 0, 0 };

constant struct plane_s scene_floor = {
  { 0, -10, 0 },
  { 0, 1, 0 },
};
constant float3 floor_color = { 0, 1, 0 };

constant float3 light_position = { 0, 20, 0 };
constant float3 light_color = { 1.0, 1.0, 1.0 };
constant float light_intensity = 1;

float lambertian_brdf(float intensity, float3 normal, float3 incident) {
  return intensity * M_1_PI * dot(normal, incident);
}

int2 get_coords_from_id(size_t id) {
  return (int2)(id % 480, id / 480);
}

// TODO: support rays originating from inside the sphere
bool ray_sphere_intersection(
    struct sphere_s sphere, struct ray_s ray, float3* intersection) {
  float3 v = ray.direction;
  float3 u = sphere.origin - ray.origin;
  float3 puv = (dot(v, u) / length(v)) * v;
  float3 projected = ray.origin + puv;
  float projected_distance_to_sphere_center = length(sphere.origin - projected);

  if(projected_distance_to_sphere_center > sphere.radius) return false;
  if(projected_distance_to_sphere_center == sphere.radius) {
    *intersection = projected;
    return true;
  }

  float other_sq = pown(length(projected - sphere.origin), 2);
  float dist_to_intersection =
      length(projected - ray.origin) - sqrt(pown(sphere.radius, 2) + other_sq);
  *intersection = ray.origin + ray.direction * dist_to_intersection;
  return true;
}

bool ray_plane_intersection(
    struct plane_s plane, struct ray_s ray, float3* intersection) {
  float dot_ln = dot(ray.direction, plane.normal);
  if(dot_ln == 0.) return false;
  float distance = dot(plane.origin - ray.origin, plane.normal) / dot_ln;
  if(distance <= 0.f) return false;
  *intersection = ray.origin + ray.direction * distance;
  return true;
}

void kernel render(write_only image2d_t output) {
  float2 film_pixel_size = {
    film_size.x / film_resolution.x,
    film_size.y / film_resolution.y
  };

  int2 coords = get_coords_from_id(get_global_id(0));
  uint x = coords.x;
  uint y = coords.y;

  float3 front = normalize(camera.look_at - camera.position);
  float3 left = normalize(cross(front, camera.up));
  float3 up = -normalize(cross(front, left));

  float3 film_origin = front * distance_to_film + camera.position;

  float3 film_intersection = film_origin +
                             left * film_size.x / 2 +
                             up * film_size.y / 2 -
                             x * film_pixel_size.x * left -
                             y * film_pixel_size.y * up;
  float3 ray_direction = normalize(film_intersection - camera.position);
  struct ray_s ray = {
    camera.position,
    ray_direction,
  };

  float4 color_sample = { 1, 1, 1, 1 };
  float3 intersection;
  if(ray_sphere_intersection(sphere, ray, &intersection)) {
    float brdf_sample =
        lambertian_brdf(light_intensity,
                        normalize(intersection - sphere.origin),
                        normalize(light_position - intersection));
    color_sample = (float4)(brdf_sample * light_color * sphere_color, 1);
  } else if(ray_plane_intersection(scene_floor, ray, &intersection)) {
    float brdf_sample =
        lambertian_brdf(light_intensity,
                        scene_floor.normal,
                        normalize(light_position - intersection));
    color_sample = (float4)(brdf_sample * light_color * floor_color, 1);
  }
  write_imagef(output, coords, color_sample);
}
