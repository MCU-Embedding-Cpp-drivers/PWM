#include "PWM.hpp"


PWM::PWM(TIM *tim, PWM_Channel channel) : _tim(tim), _channel(channel) {
}


ErrorCode PWM::Init() {
	_tim->Init();
	setCCRptr();

	return ErrorCode::OK;
}

TIM *PWM::getTIM() {
	return _tim;
}

/**
 * @brief 设置 PWM 占空比
 * @param duty_cycle 占空比，范围 [0.0, 100.0]，0.0 = 0%，100.0 = 100%
 * @return ErrorCode
 *   - OK: 设置成功
 *   - Fail: 句柄无效、通道不支持或占空比越界
 * @note 通过修改CCRx实现
 */
ErrorCode PWM::setDutyCycle(float duty_cycle) {

	if (duty_cycle < 0.0f) {
		return ErrorCode::Fail;
	} else if (duty_cycle > 100.0f) { // 防止浮点误差导致超出范围
		duty_cycle = 100.0;
	}

	// 获取当前周期（ARR + 1）
	uint32_t period = _tim->getPeriod();
	if (period == 0) {
		return ErrorCode::Fail; // 未配置频率或定时器未启动
	}

	// 计算 CCR 值：CCR = (duty_cycle / 100) * period
	uint32_t ccr = static_cast<uint32_t>((duty_cycle / 100.0f) * period);

	// 防止浮点误差导致超出范围
	if (ccr > period) {
		ccr = period;
	}

	switch (_channel) {
		case TIM::CH1:
			_tim->getHALHandle()->Instance->CCR1 = ccr;
			break;
		case TIM::CH2:
			_tim->getHALHandle()->Instance->CCR2 = ccr;
			break;
		case TIM::CH3:
			_tim->getHALHandle()->Instance->CCR3 = ccr;
			break;
		case TIM::CH4:
			_tim->getHALHandle()->Instance->CCR4 = ccr;
			break;
		case TIM::CH5:
			_tim->getHALHandle()->Instance->CCR5 = ccr;
			break;
		case TIM::CH6:
			_tim->getHALHandle()->Instance->CCR6 = ccr;
			break;
		default:
			return ErrorCode::Fail;
	}

	return ErrorCode::OK;
}

ErrorCode PWM::setAutoFrequency(float hz) {
	ErrorCode err = _tim->setAutoFrequency(hz);
	return err;
}

/**
 * @brief 自动设置脉宽（基于微秒）
 * @param us 脉宽值（微秒，支持小数）
 * @return ErrorCode
 * @note 最小脉宽 1 / bus_clock_hz 秒，通过PSC=65535+ARR=1实现
 */
ErrorCode PWM::setAutoPulseWidth(float us) {
	// 获取总线时钟频率（Hz）
	uint32_t bus_clock_hz = _tim->getBusClockFrequency();

	// 验证输入有效性
	if (us * 1000 < UnitConverter::Time::s2ns(1 / bus_clock_hz)) {
		return ErrorCode::InvalidParam;
	}

	for (uint32_t psc = 1; psc <= 65535; ++psc) {
		// 计算ARR值：ARR = (bus_clock_hz * us) / (psc * 1e6)
		uint32_t arr_val = static_cast<uint32_t>((bus_clock_hz * us) / (psc * 1000000U));

		if (arr_val > 65535) {
			continue; // 超出范围跳过
		}

		// 设置定时器参数
		_tim->setPSR(psc - 1);
		_tim->setARR(arr_val);

		// 设置占空比50%
		return _tim->setCCR(arr_val / 2, _channel);
	}

	return ErrorCode::Fail; // 无合适配置
}

ErrorCode PWM::setARR(uint32_t arr) {
	_tim->setARR(arr);
	return ErrorCode::OK;
}

ErrorCode PWM::setCCR(uint32_t ccr) {
	_tim->setCCR(ccr, _channel);
	return ErrorCode::OK;
}

ErrorCode PWM::setPSR(uint16_t psr) {
	_tim->setPSR(psr);
	return ErrorCode::OK;
}

ErrorCode PWM::Enable() {
	if (HAL_TIM_PWM_Start(_tim->getHALHandle(), static_cast<uint32_t>(_channel)) != HAL_OK) {
		return ErrorCode::Fail;
	}

	return ErrorCode::OK;
}

ErrorCode PWM::Disable() {
	if (HAL_TIM_PWM_Stop(_tim->getHALHandle(), static_cast<uint32_t>(_channel)) != HAL_OK) {
		return ErrorCode::Fail;
	}

	return ErrorCode::OK;
}

ErrorCode PWM::setCCRptr() {
	switch (_channel) {
		case PWM_Channel::CH1:
			_ccr_ptr = &(_tim->getHALHandle()->Instance->CCR1);
			break;

		case PWM_Channel::CH2:
			_ccr_ptr = &(_tim->getHALHandle()->Instance->CCR2);
			break;

		case PWM_Channel::CH3:
			_ccr_ptr = &(_tim->getHALHandle()->Instance->CCR3);
			break;

		case PWM_Channel::CH4:
			_ccr_ptr = &(_tim->getHALHandle()->Instance->CCR4);
			break;

		case PWM_Channel::CH5:
			_ccr_ptr = &(_tim->getHALHandle()->Instance->CCR5);
			break;

		case PWM_Channel::CH6:
			_ccr_ptr = &(_tim->getHALHandle()->Instance->CCR6);
			break;

		default:
			return ErrorCode::Fail;
			break;
	}
	return ErrorCode::OK;
}
