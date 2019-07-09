/*
 * Copyright (C) 2017 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

package org.strongswan.android.ui.widget;

import android.content.Context;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.widget.Checkable;
import android.widget.LinearLayout;

public class CheckableLinearLayout extends LinearLayout implements Checkable
{
	private static final int[] CHECKED_STATE_SET = {android.R.attr.state_checked};
	private boolean mChecked;

	public CheckableLinearLayout(Context context, @Nullable AttributeSet attrs)
	{
		super(context, attrs);
	}

	@Override
	public void setChecked(boolean checked)
	{
		if (mChecked != checked)
		{
			mChecked = checked;
			refreshDrawableState();
		}
	}

	@Override
	public boolean isChecked()
	{
		return mChecked;
	}

	@Override
	public void toggle()
	{
		setChecked(!mChecked);
	}

	@Override
	protected int[] onCreateDrawableState(int extraSpace)
	{
		final int[] drawableState = super.onCreateDrawableState(extraSpace + 1);
		if (isChecked())
		{
			mergeDrawableStates(drawableState, CHECKED_STATE_SET);
		}
		return drawableState;
	}
}
