mat3 getTBN(vec3 localNormal, vec3 worldPos, vec2 uv) {
  vec3 q1 = dFdx(worldPos);
  vec3 q2 = dFdy(worldPos);
  vec2 st1 = dFdx(uv);
  vec2 st2 = dFdy(uv);

  vec3 N = localNormal;
  vec3 T = normalize(q1 * st2.t - q2 * st1.t);
  vec3 B = -normalize(cross(N, T));

  return mat3(T, B, N);
}

