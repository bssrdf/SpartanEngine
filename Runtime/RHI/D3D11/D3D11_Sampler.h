/*
Copyright(c) 2016-2018 Panos Karabelas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

//= INCLUDES ===============
#include "../IRHI_Sampler.h"
//==========================

namespace Directus
{
	class ENGINE_CLASS D3D11_Sampler : public IRHI_Sampler
	{
	public:
		D3D11_Sampler(
			RHI_Device* rhiDevice, 
			Texture_Sampler_Filter filter					= Texture_Sampler_Anisotropic,
			Texture_Address_Mode textureAddressMode			= Texture_Address_Wrap,
			Texture_Comparison_Function comparisonFunction	= Texture_Comparison_Always
		);
		~D3D11_Sampler();

		bool Bind(unsigned int slot) override;

	private:
		RHI_Device* m_rhiDevice;
		ID3D11SamplerState* m_samplerState;
	};
}