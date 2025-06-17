cbuffer wvp : register(b0)
{
	matrix wvp;
}

struct vertex_in
{
	float3 pos : POSITION;
	float4 col : COLOR0;
};

struct vertex_out
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
};

vertex_out vs_main(vertex_in input)
{
	vertex_out output;
	output.pos = mul(wvp, float4(input.pos, 1.0f));
	output.col = input.col;
	// output.col = float4(0.0f, 1.0f, 0.0f, 1.0f);
	return output;
}

float4 ps_main(vertex_out input) : SV_TARGET {
	return input.col;
}