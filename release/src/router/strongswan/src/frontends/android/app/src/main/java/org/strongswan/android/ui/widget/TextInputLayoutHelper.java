/*
 * Copyright (C) 2016 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

package org.strongswan.android.ui.widget;

import android.content.Context;
import android.content.res.TypedArray;
import android.support.annotation.Nullable;
import android.support.design.widget.TextInputLayout;
import android.support.v4.view.ViewCompat;
import android.support.v4.view.ViewPropertyAnimatorListenerAdapter;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.strongswan.android.R;

/**
 * Layout that extends {@link android.support.design.widget.TextInputLayout} with a helper text
 * displayed below the text field when it receives the focus. Also, any error message shown with
 * {@link #setError(CharSequence)} is hidden when the text field is changed (this mirrors the
 * behavior of {@link android.widget.EditText}).
 */
public class TextInputLayoutHelper extends TextInputLayout
{
	private LinearLayout mHelperContainer;
	private TextView mHelperText;

	public TextInputLayoutHelper(Context context)
	{
		this(context, null);
	}

	public TextInputLayoutHelper(Context context, AttributeSet attrs)
	{
		this(context, attrs, 0);
	}

	public TextInputLayoutHelper(Context context, AttributeSet attrs, int defStyleAttr)
	{
		super(context, attrs);
		TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.TextInputLayoutHelper);
		String helper = a.getString(R.styleable.TextInputLayoutHelper_helper_text);
		a.recycle();
		if (helper != null)
		{
			mHelperContainer = new LinearLayout(context);
			mHelperContainer.setOrientation(LinearLayout.HORIZONTAL);
			mHelperContainer.setVisibility(View.INVISIBLE);
			addView(mHelperContainer, LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);

			mHelperText = new TextView(context);
			mHelperText.setText(helper);
			mHelperText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
			a = context.obtainStyledAttributes(attrs, new int[]{android.R.attr.textColorSecondary});
			mHelperText.setTextColor(a.getColor(0, mHelperText.getCurrentTextColor()));
			a.recycle();

			mHelperContainer.addView(mHelperText, LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
		}
	}

	@Override
	public void addView(View child, int index, ViewGroup.LayoutParams params)
	{
		super.addView(child, index, params);
		if (child instanceof EditText)
		{
			EditText text = (EditText)child;
			text.addTextChangedListener(new TextWatcher() {
				@Override
				public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

				@Override
				public void onTextChanged(CharSequence s, int start, int before, int count) {}

				@Override
				public void afterTextChanged(Editable s)
				{
					if (getError() != null)
					{
						setError(null);
					}
				}
			});
			if (mHelperContainer != null)
			{
				text.setOnFocusChangeListener(new OnFocusChangeListener() {
					@Override
					public void onFocusChange(View v, boolean hasFocus)
					{
						showHelper(hasFocus);
					}
				});
				ViewCompat.setPaddingRelative(mHelperContainer, ViewCompat.getPaddingStart(text),
											  0, ViewCompat.getPaddingEnd(text), text.getPaddingBottom());
			}
		}
	}

	@Override
	public void setError(@Nullable CharSequence error)
	{
		super.setError(error);
		if (mHelperContainer != null)
		{
			if (error == null)
			{	/* this frees up space used by the now invisible error message */
				setErrorEnabled(false);
			}
			else
			{	/* re-add the helper as the error message should be displayed directly under the textbox */
				removeView(mHelperContainer);
				addView(mHelperContainer, LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
			}
		}
	}

	/**
	 * Set the helper text to be displayed below the text field.
	 *
	 * @attr ref R.styleable#TextInputLayoutHelper_helper_text
	 */
	public void setHelperText(CharSequence text)
	{
		mHelperText.setText(text);
	}

	private void showHelper(boolean show)
	{
		if (show == (mHelperContainer.getVisibility() == View.VISIBLE))
		{
			return;
		}
		if (show)
		{
			ViewCompat.animate(mHelperContainer)
					  .alpha(1f)
					  .setDuration(200)
					  .setListener(new ViewPropertyAnimatorListenerAdapter() {
						  @Override
						  public void onAnimationStart(View view)
						  {
							  view.setVisibility(View.VISIBLE);
						  }
					  }).start();
		}
		else
		{
			ViewCompat.animate(mHelperContainer)
					  .alpha(0f)
					  .setDuration(200)
					  .setListener(new ViewPropertyAnimatorListenerAdapter() {
						  @Override
						  public void onAnimationEnd(View view)
						  {
							  view.setVisibility(View.INVISIBLE);
						  }
					  }).start();
		}
	}
}
