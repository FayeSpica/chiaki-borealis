// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

package com.metallic.chiaki.common

import android.app.UiModeManager
import android.content.Context
import android.content.res.Configuration

object TvUtils
{
	fun isTV(context: Context): Boolean
	{
		val uiModeManager = context.getSystemService(Context.UI_MODE_SERVICE) as UiModeManager
		return uiModeManager.currentModeType == Configuration.UI_MODE_TYPE_TELEVISION
	}
}
