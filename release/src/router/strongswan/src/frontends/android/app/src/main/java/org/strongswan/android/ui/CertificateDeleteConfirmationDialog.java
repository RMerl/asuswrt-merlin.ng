/*
 * Copyright (C) 2014 Tobias Brunner
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

package org.strongswan.android.ui;

import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatDialogFragment;

import org.strongswan.android.R;

/**
 * Class that displays a confirmation dialog to delete a selected local
 * certificate.
 */
public class CertificateDeleteConfirmationDialog extends AppCompatDialogFragment
{
	public static final String ALIAS = "alias";
	OnCertificateDeleteListener mListener;

	/**
	 * Interface that can be implemented by parent activities to get the
	 * alias of the certificate to delete, if the user confirms the deletion.
	 */
	public interface OnCertificateDeleteListener
	{
		public void onDelete(String alias);
	}

	@Override
	public void onAttach(Context context)
	{
		super.onAttach(context);
		if (context instanceof OnCertificateDeleteListener)
		{
			mListener = (OnCertificateDeleteListener)context;
		}
	}

	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState)
	{
		return new AlertDialog.Builder(getActivity())
			.setIcon(android.R.drawable.ic_dialog_alert)
			.setTitle(R.string.delete_certificate_question)
			.setMessage(R.string.delete_certificate)
			.setPositiveButton(R.string.delete_profile, new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int whichButton)
				{
					if (mListener != null)
					{
						mListener.onDelete(getArguments().getString(ALIAS));
					}
				}
			})
			.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which)
				{
					dismiss();
				}
			}).create();
	}
}
