float sRGB_To_Linear_Single( float sRGB_color_channel )
{
    float comparison_result = float( sRGB_color_channel <= 0.04045f );
    return ( comparison_result * sRGB_color_channel / 12.92f ) + ( ( 1.0f - comparison_result )  * pow( ( sRGB_color_channel + 0.055f ) / 1.055f, 2.4f ) );
}

vec4 sRGB_To_Linear( vec4 sRGB_color )
{
    
    return vec4( sRGB_To_Linear_Single( sRGB_color.x ),
                 sRGB_To_Linear_Single( sRGB_color.y ),
                 sRGB_To_Linear_Single( sRGB_color.z ),
                 sRGB_color.w );
}

float Linear_To_sRGB_Single( float linear_color_channel )
{
    float comparison_result = float( linear_color_channel <= 0.0031308f );
    return ( comparison_result * linear_color_channel * 12.92f ) + ( ( 1.0f - comparison_result ) * ( 1.055f * pow( linear_color_channel, 1.0f / 2.4f ) - 0.055f ) );
}

vec4 Linear_To_sRGB( vec4 linear_color )
{
    
    return vec4( Linear_To_sRGB_Single( linear_color.x ),
                 Linear_To_sRGB_Single( linear_color.y ),
                 Linear_To_sRGB_Single( linear_color.z ),
                 linear_color.w );
}