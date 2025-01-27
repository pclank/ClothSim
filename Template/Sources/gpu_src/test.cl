float4 lerp(float4 a, float4 b, float t)
{
	return (1.0f - t) * a + t * b;
}

kernel void test(__global int* test_buf)
{
	int x = get_global_id(0);
	//int y = get_global_id(1);

	test_buf[x] = x;
}

kernel void tex_test(write_only image2d_t tgt_tex)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	write_imagef(tgt_tex, (int2)(x, y), (float4)(0.001f * x, 0.001f * y, 0.0f, 1.0f));
	//write_imagef(tgt_tex, (int2)(x, y), (float4)(1.0f, 0.0f, 0.0f, 1.0f));
}

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

kernel void tex_read_test(read_only image2d_t tgt_tex, __global float* debug_buf)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	float4 pixel = read_imagef(tgt_tex, sampler, (int2)(x, y));

	debug_buf[x + y * get_image_width(tgt_tex)] = pixel.x;
}

kernel void AvectFluid(float timestep, float rdx,
	// 1 / grid scale
	read_only image2d_t u,		// input velocity
	read_only image2d_t xOld,	// qty to advect
	write_only image2d_t xNew	// advected qty
)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	// follow the velocity field "back in time"
	float2 pos = (float2)(coords.x, coords.y) - timestep * rdx * read_imagef(u, sampler, coords).xy;

	// find 4 closest texel positions
	float4 st;

	st.xy = floor(pos - 0.5f) + 0.5f;
	st.zw = st.xy + 1.0f;

	float2 t = pos - st.xy;

	float4 tex11 = read_imagef(xOld, sampler, st.xy);
	float4 tex21 = read_imagef(xOld, sampler, st.zy);
	float4 tex12 = read_imagef(xOld, sampler, st.xw);
	float4 tex22 = read_imagef(xOld, sampler, st.zw);

	// bilinearly interpolate
	float4 interpolated = lerp(lerp(tex11, tex21, t.x), lerp(tex12, tex22, t.x), t.y);

	write_imagef(xNew, coords, interpolated);
}
