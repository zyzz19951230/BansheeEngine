#pragma once

#include "CmPrerequisites.h"
#include "CmRTTIType.h"
#include "CmSamplerState.h"
#include "CmRenderStateManager.h"

namespace CamelotEngine
{
	class CM_EXPORT SamplerStateRTTI : public RTTIType<SamplerState, IReflectable, SamplerStateRTTI>
	{
	private:
		SAMPLER_STATE_DESC& getData(SamplerState* obj) { return obj->mData; }
		void setData(SamplerState* obj, SAMPLER_STATE_DESC& val) 
		{ 
			obj->mRTTIData = val;
		} 

	public:
		SamplerStateRTTI()
		{
			addPlainField("mData", 0, &SamplerStateRTTI::getData, &SamplerStateRTTI::setData);
		}

		virtual void onDeserializationEnded(IReflectable* obj)
		{
			SamplerState* samplerState = static_cast<SamplerState*>(obj);
			if(!samplerState->mRTTIData.empty())
			{
				SAMPLER_STATE_DESC desc = boost::any_cast<SAMPLER_STATE_DESC>(samplerState->mRTTIData);

				samplerState->initialize(desc);
			}

		}

		virtual const String& getRTTIName()
		{
			static String name = "SamplerState";
			return name;
		}

		virtual UINT32 getRTTIId()
		{
			return TID_SamplerState;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject()
		{
			return RenderStateManager::instance().createEmptySamplerState();
		}
	};
}