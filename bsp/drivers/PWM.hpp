#ifndef __PWM_HPP
#define __PWM_HPP

#include "System.hpp"
#include "SystemTypes.hpp"
#include "TIM.hpp"
#include "UnitConverter.hpp"


class PWM // 使用时需要先注册对应定时器
{
  public:
	/**
	 * PWM 输出极性
	 */
	enum class PWM_Polarity {
		HIGH = TIM_OCPOLARITY_HIGH, // 高电平有效（默认）
		LOW	 = TIM_OCPOLARITY_LOW	// 低电平有效
	};

	/**
	 * PWM 模式
	 */
	enum class PWM_Mode {
		PWM1 = TIM_OCMODE_PWM1, // PWM 模式1：计数器 < CCR → 高，否则低
		PWM2 = TIM_OCMODE_PWM2	// PWM 模式2：计数器 < CCR → 低，否则高
	};

	using PWM_Channel = TIM::TIM_Channel;

	explicit PWM(TIM* tim, PWM_Channel channel); // 推荐命名 PWMx_Chx	(定时器x,通道x)

	ErrorCode Init();

	TIM* getTIM();

	/**
	 * @brief 设置 PWM 占空比
	 * @param duty_cycle 占空比，范围 [0.0, 100.0]，0.0 = 0%，100.0 = 100%
	 * @return ErrorCode
	 *   - OK: 设置成功
	 *   - Fail: 句柄无效、通道不支持或占空比越界
	 * @note 通过修改CCRx实现
	 */
	ErrorCode setDutyCycle(float duty_cycle);

	/**
	 * @brief 自动配置频率
	 * @param hz
	 * @return ErrorCode
	 * @note 自动配置PSR和ARR
	 */
	ErrorCode setAutoFrequency(float hz);

	/**
	 * @brief 自动设置脉宽（基于微秒）
	 * @param us 脉宽值（微秒），支持浮点数精度
	 * @return ErrorCode
	 *   - OK: 成功配置指定脉宽
	 *   - InvalidParam: us < 1 / 总线频率（低于硬件最小粒度）
	 *   - Fail: 无合法PSC/ARR寄存器组合
	 * @note 1. 动态计算ARR = (bus_clock_hz * us) / (psc * 1e6)
	 *       2. 自动匹配PSC/ARR寄存器（PSC ∈ [0,65535], ARR ∈ [0,65535]）
	 *       3. 固定占空比50%（通过setCCR(arr_val / 2, _channel)）
	 *       4. 实际频率由getBusClockFrequency()动态获取
	 */
	ErrorCode setAutoPulseWidth(float us);

	ErrorCode setARR(uint32_t arr);

	ErrorCode setCCR(uint32_t ccr);

	ErrorCode setPSR(uint16_t psr);

	// 使能输出
	ErrorCode Enable();

	// 失能输出
	ErrorCode Disable();

  private:
	TIM*			 _tim; // 组合 TIM 实例（非继承）
	TIM::TIM_Channel _channel;
	volatile uint32_t* _ccr_ptr;

	ErrorCode setCCRptr();
};

#endif // __PWM_HPP
