#pragma once

#pragma region SafeRelease/SafeDelete
#define SafeRelease(pInterface)\
	if(pInterface != nullptr)\
	{\
		pInterface->Release();\
		pInterface = nullptr;\
	}

#define PxSafeRelease(pInterface)\
	if(pInterface != nullptr)\
	{\
		pInterface->release();\
		pInterface = nullptr;\
	}

#define SafeDelete(pObject)\
	if(pObject != nullptr)\
	{\
		delete pObject;\
		pObject = nullptr;\
	}
#pragma endregion

#pragma region Enum & Bitfield
template<typename Enum>
struct EnableBitMaskOperators
{
	static const bool enable = false;
};

template<typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::enable, Enum>
operator |(Enum lhs, Enum rhs)
{
	using underlying = std::underlying_type_t<Enum>;
	return static_cast<Enum> (
		static_cast<underlying>(lhs) |
		static_cast<underlying>(rhs)
		);
}

template<typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::enable, Enum>
operator &(Enum lhs, Enum rhs)
{
	using underlying = std::underlying_type_t<Enum>;
	return static_cast<Enum> (
		static_cast<underlying>(lhs) &
		static_cast<underlying>(rhs)
		);
}

template<typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::enable, Enum>
operator &=(Enum& lhs, Enum rhs)
{
	using underlying = std::underlying_type_t<Enum>;
	lhs = static_cast<Enum> (
		static_cast<underlying>(lhs) &
		static_cast<underlying>(rhs)
		);

	return lhs;
}

template<typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::enable, Enum>
operator |=(Enum& lhs, Enum rhs)
{
	using underlying = std::underlying_type_t<Enum>;
	lhs = static_cast<Enum> (
		static_cast<underlying>(lhs) |
		static_cast<underlying>(rhs)
		);

	return lhs;
}

template<typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::enable, Enum>
operator ~(Enum lhs)
{
	using underlying = std::underlying_type_t<Enum>;
	lhs = static_cast<Enum> (
		~static_cast<underlying>(lhs));

	return lhs;
}

template<typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::enable, bool>
isSet(Enum lhs, Enum rhs)
{
	return (lhs & rhs) == rhs;
}

#define ENABLE_BITMASK_OPERATORS(x)  \
template<>                           \
struct EnableBitMaskOperators<x>     \
{                                    \
    static const bool enable = true; \
};									 

#pragma endregion

#pragma region HALT
#ifdef _DEBUG
#define HALT() DebugBreak();
#else
#define HALT() exit(-1);
#endif
#pragma endregion

#pragma region Error Checking
#define ASSERT_IF_(isTrue)\
	if(isTrue){HANDLE_ERROR(L"[ASSERT]" #isTrue);}\
	_Analysis_assume_(!(isTrue));

#define ASSERT_IF(isTrue, ...)\
	if(isTrue){HANDLE_ERROR(__VA_ARGS__);}\
	_Analysis_assume_(!(isTrue));

#define ASSERT_NULL_(object)\
	if((object) == nullptr){HANDLE_ERROR(L"[ASSERT]" #object L" == nullptr");}\
	_Analysis_assume_((object) != nullptr);

#define ASSERT_NULL(object, ...)\
	if((object) == nullptr){HANDLE_ERROR(__VA_ARGS__);}\
	_Analysis_assume_((object) != nullptr);

#define HANDLE_ERROR(...)\
	if(Logger::LogError(__VA_ARGS__)) HALT()

#define HANDLE_ERROR_INFO(fmt, info, ...)\
	if(Logger::LogError({fmt, info}, __VA_ARGS__)) HALT()

#define HANDLE_ERROR_LOC(fmt, loc, ...)\
	if(Logger::LogError({fmt, loc}), __VA_ARGS__) HALT()
#pragma endregion

#pragma region Query EffectVariable
#define QUERY_EFFECT_VARIABLE_HALT(effectParam, varParam, varName, varType)			\
varParam = (effectParam)->GetVariableByName(#varName)->As##varType();	\
if(!(varParam)->IsValid())											\
{																	\
	const auto className = StringUtil::utf8_decode(typeid(*this).name()).substr(6); /*Remove 'class ' prefix*/ \
	HANDLE_ERROR(L"[{}] Effect Variable \"{}\" not found.\n", className, L#varName); \
}
#pragma endregion