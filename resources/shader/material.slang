struct VertexOutput
{
};

[shader("vertex")]
VertexOutput vertex_main()
{
    VertexOutput output;

    return output;
}

struct FragmentOutput
{
    float4 color;
};

[shader("fragment")]
FragmentOutput fragment_main() : SV_Target
{
    FragmentOutput output;
    output.color = float4(1.0, 1.0, 1.0, 1.0);
    return output;
}
