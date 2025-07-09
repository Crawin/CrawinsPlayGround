//게임 객체의 정보를 위한 상수 버퍼를 선언한다.
cbuffer cbGameObjectInfo : register(b0)
{
    matrix gmtxWorld : packoffset(c0);
};

//카메라의 정보를 위한 상수 버퍼를 선언한다.
cbuffer cbCameraInfo : register(b1)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProjection : packoffset(c4);
};

//정점 셰이더의 입력을 위한 구조체를 선언한다. 
struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
};

//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다. 
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};
//정점 셰이더를 정의한다. 
VS_OUTPUT VSDiffused(VS_INPUT input)
{
    VS_OUTPUT output;
    //정점을 변환(월드 변환, 카메라 변환, 투영 변환)한다. 
    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
    output.color = input.color;
    return (output);
}

//픽셀 셰이더를 정의한다. 
float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
{
    return (input.color);
}

//정점 데이터와 인스턴싱 데이터를 위한 구조체이다. 
struct VS_INSTANCING_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float4x4 mtxTransform : WORLDMATRIX;
    float4 instanceColor : INSTANCECOLOR;
};
struct VS_INSTANCING_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VS_INSTANCING_OUTPUT VSInstancingIL(VS_INSTANCING_INPUT input)
{
    VS_INSTANCING_OUTPUT output;
    output.position = mul(mul(mul(float4(input.position, 1.0f), input.mtxTransform), gmtxView), gmtxProjection);
    output.color = input.color + input.instanceColor;
    //output.color = input.color;
    return (output);
}
float4 PSInstancingIL(VS_INSTANCING_OUTPUT input) : SV_TARGET
{
    return (input.color);
}




struct VS_GSCube_IN
{
    float3 position : POSITION;
    float angle : ANGLE;
    float pad : PADDING;
};

struct GS_OUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
};


VS_GSCube_IN VS_GS(VS_GSCube_IN input)
{
    return input;
}

[maxvertexcount(24)]
void GS_Cube(point VS_GSCube_IN input[1], inout TriangleStream<GS_OUT> triStream)
{
    float3 center = input[0].position;
    float pad = input[0].pad;
    float3 vertices[8] =
    {
        center + float3(-pad, -pad, -pad),  // 0 전면 좌하단
        center + float3(-pad, pad, -pad),   // 1 전면 좌상단
        center + float3(pad, pad, -pad),    // 2 전면 우상단
        center + float3(pad, -pad, -pad),   // 3 전면 우하단
        center + float3(-pad, -pad, pad),   // 4 후면 좌하단
        center + float3(-pad, pad, pad),    // 5 후면 좌상단
        center + float3(pad, pad, pad),     // 6 후면 우상단
        center + float3(pad, -pad, pad),    // 7 후면 우하단
    };

    int indices[24] =
    {
        1,2,0,3,
        2,6,3,7,
        6,5,7,4,
        5,1,4,0,
        5,6,1,2,
        0,3,4,7,
    };

    for (int i = 0; i < 6; ++i)
    {
        GS_OUT out0, out1, out2, out3;
        matrix ViewProj = mul(gmtxView, gmtxProjection);
        out0.position = mul(float4(vertices[indices[i * 4 + 0]], 1.0f), ViewProj);
        out1.position = mul(float4(vertices[indices[i * 4 + 1]], 1.0f), ViewProj);
        out2.position = mul(float4(vertices[indices[i * 4 + 2]], 1.0f), ViewProj);
        out3.position = mul(float4(vertices[indices[i * 4 + 3]], 1.0f), ViewProj);

        out0.color = float4(1, 1, 1, 1);
        out1.color = float4(1, 1, 1, 1);
        out2.color = float4(0, 0, 0, 1);
        out3.color = float4(0, 0, 0, 1);

        triStream.Append(out0);
        triStream.Append(out1);
        triStream.Append(out2);
        triStream.Append(out3);
        triStream.RestartStrip();
    }

}

[maxvertexcount(24)]
void GS_Cube_Animation(point VS_GSCube_IN input[1], inout TriangleStream<GS_OUT> triStream)
{
    float3 center = input[0].position;
    float pad = input[0].pad;
    float angle = input[0].angle;
    float3 vertices[8] =
    {
        float3(-pad, -pad, -pad), // 0 전면 좌하단
        float3(-pad, pad, -pad), // 1 전면 좌상단
        float3(pad, pad, -pad), // 2 전면 우상단
        float3(pad, -pad, -pad), // 3 전면 우하단
        float3(-pad, -pad, pad), // 4 후면 좌하단
        float3(-pad, pad, pad), // 5 후면 좌상단
        float3(pad, pad, pad), // 6 후면 우상단
        float3(pad, -pad, pad), // 7 후면 우하단
    };

    int indices[24] =
    {
        1, 2, 0, 3,
        2, 6, 3, 7,
        6, 5, 7, 4,
        5, 1, 4, 0,
        5, 6, 1, 2,
        0, 3, 4, 7,
    };

    for (int i = 0; i < 6; ++i)
    {
        GS_OUT out0, out1, out2, out3;
        matrix Rotate =
        {
            cos(angle), 0, -sin(angle), 0,
            0, 1, 0, 0,
            sin(angle), 0, cos(angle), 0,
            0,          0,          0, 1
        };
        matrix World =
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            center.x, center.y, center.z, 1
        };
        matrix Trans = mul(Rotate, World);
        matrix ViewProj = mul(mul(Trans, gmtxView), gmtxProjection);
        out0.position = mul(float4(vertices[indices[i * 4 + 0]], 1.0f), ViewProj);
        out1.position = mul(float4(vertices[indices[i * 4 + 1]], 1.0f), ViewProj);
        out2.position = mul(float4(vertices[indices[i * 4 + 2]], 1.0f), ViewProj);
        out3.position = mul(float4(vertices[indices[i * 4 + 3]], 1.0f), ViewProj);

        out0.color = float4(1, 1, 1, 1);
        out1.color = float4(1, 1, 1, 1);
        out2.color = float4(0, 0, 0, 1);
        out3.color = float4(0, 0, 0, 1);

        triStream.Append(out0);
        triStream.Append(out1);
        triStream.Append(out2);
        triStream.Append(out3);
        triStream.RestartStrip();
    }

}

float4 PS_GSCube(GS_OUT input):SV_Target
{
    return input.color;
}