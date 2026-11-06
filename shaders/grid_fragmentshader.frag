#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 vNearWorld;
layout (location = 1) in vec4 vFarWorld;
layout (location = 2) in mat4 viewProjMatrix;

layout (location = 0) out vec4 outColor;

/* ------------------------- 可调参数 ------------------------- */
const float SPACING_FINE   = 1.0;
const float SPACING_COARSE = 10.0;

/* 线的半宽, 单位 = 屏幕像素 (1.0 => 线宽约 2 像素) */
const float LINE_HALF_WIDTH_PIXELS = 1.6;
const float LINE_CORE_RATIO        = 0.55;  /* 核心(满亮度)占半宽的比例 */

/* 梯形剖面: |x|<=c 时 1.0, c<|x|<t 时线性降到 0。单周期面积 = t + c */
float TrapArea(float d, float t, float c)
{
    if (d <= c) { return d; }
    if (d >= t) { return 0.5 * (t + c); }
    return d - (d - c) * (d - c) / (2.0 * max(t - c, 1e-6));
}

/* 覆盖率函数 f(x) = 1 - |x|/t (|x| < t), 否则 0
   下面是它从 0 到 x 的原函数, 闭式解, 无分支跳变
   t = 线的半宽(格子单位), 必须 < 0.5 */
float LineIntegral(float x, float t, float c)
{
	float Period = floor(x);
	float r = x - Period;						/* r ∈ [0,1) */

	 float Within = (r <= 0.5)
        ? TrapArea(r, t, c)
        : ((t + c) - TrapArea(1.0 - r, t, c));

	return Period * (t + c) + Within;			/* 每个整周期贡献面积 t */
}

/* 盒式滤波: 这个像素的足迹 [u-w/2, u+w/2] 内, 有多大比例落在线上
   输出是 (u, w) 的连续函数; w 越大越接近"平均覆盖率", 与亚像素相位解耦 */
float FilteredLine(float u, float t, float c, float w)
{
	float HalfW = max(w, 1e-4) * 0.5;
	return (LineIntegral(u + HalfW, t, c) - LineIntegral(u - HalfW, t, c)) / (2.0 * HalfW);
}

/* 一个尺度的网格: x/y 两族线各自做 1D 滤波, 然后取并集(用 max 避免交点过曝) */
float GridCoverage(vec2 U, vec2 W)
{
	vec2 T = clamp(W * LINE_HALF_WIDTH_PIXELS, 1e-5, 0.5);
	vec2 C = T * LINE_CORE_RATIO;
	return max(FilteredLine(U.x, T.x, C.x, W.x), FilteredLine(U.y, T.y, C.y, W.y));
}

void main()
{
	/* ---------- 1. 射线: 全部无条件计算 (discard 一律挪到最后) ---------- */
	vec3 NearPoint = vNearWorld.xyz / vNearWorld.w;
	vec3 FarPoint  = vFarWorld.xyz  / vFarWorld.w;
	vec3 RayDir    = FarPoint - NearPoint;

	float T = (abs(RayDir.z) > 1e-8) ? (-NearPoint.z / RayDir.z) : -1.0;
	bool bHit = (T > 0.0 && T < 1.0);

	/* 用 clamp 后的 T: 即使没打中平面也给出有限坐标,
	   这样相邻像素的 fwidth 不会拿到 inf/nan */
	vec3 PlanePoint = NearPoint + clamp(T, 0.0, 1.0) * RayDir;

	vec2 RawXY = PlanePoint.xy;

	/* 用原始坐标求导: 即使溢出到 1e30, fwidth 也只会是 inf, 下面兜住 */
	vec2 RawFootprint = fwidth(RawXY);
	if (any(isnan(RawFootprint)) || any(isinf(RawFootprint)))
	{
		RawFootprint = vec2(1e4);
	}
	
	/* ----------------------- 2. 每像素足迹 --------------------------- */
	vec2 Footprint = clamp(RawFootprint, vec2(1e-6), vec2(1e4));

	/* clamp 只用于模式求值 */
	vec2 Coord = clamp(RawXY, vec2(-65536.0), vec2(65536.0));

	/* ---------- 3. 两个尺度 ---------- */
	vec2 UFine   = Coord / SPACING_FINE;
	vec2 UCoarse = Coord / SPACING_COARSE;
	vec2 WFine   = Footprint / SPACING_FINE;
	vec2 WCoarse = Footprint / SPACING_COARSE;

	float Fine   = GridCoverage(UFine,   WFine);
	float Coarse = GridCoverage(UCoarse, WCoarse);

	/* ---------- 4. 两个尺度都要 LOD 淡出 (原来只有 Fine 有) ---------- */
	float LodFine   = max(WFine.x,   WFine.y);
	float LodCoarse = max(WCoarse.x, WCoarse.y);
	float FadeFine   = 1.0 - smoothstep(0.18, 0.40, LodFine);
	float FadeCoarse = 1.0 - smoothstep(0.18, 0.40, LodCoarse);

	float Alpha = Fine * 0.3 * FadeFine + Coarse * 0.6 * FadeCoarse;
	Alpha = pow(Alpha, 0.8);   /* 提暗部, 压缩对比, 运动时不容易被平均掉 */

	/* ---------- 5. 远景淡出, 避免地平线硬边 ---------- */
	Alpha *= 1.0 - smoothstep(0.55, 1.0, T);

	/* ---------- 6. 坐标轴染色 ---------- */
	vec3 Color = vec3(0.62);
	vec2 AxisDistance = abs(Coord) / Footprint;
	if (AxisDistance.y < 1.5) { Color = vec3(0.85, 0.22, 0.22); }
	if (AxisDistance.x < 1.5) { Color = vec3(0.25, 0.82, 0.30); }

	/* ---------- 7. 深度 ---------- */
	vec4 Clip = viewProjMatrix * vec4(PlanePoint, 1.0);
	gl_FragDepth = clamp(Clip.z / Clip.w, 0.0, 1.0);

	/* ---------- 8. 唯一的 discard 出口, 在所有导数算完之后 ---------- */
	if (!bHit || Alpha < 0.002) { discard; }

	outColor = vec4(Color, Alpha);
}