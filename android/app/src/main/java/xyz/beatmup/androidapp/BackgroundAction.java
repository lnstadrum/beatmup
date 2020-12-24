/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package xyz.beatmup.androidapp;

import android.app.Dialog;
import android.app.ProgressDialog;
import android.os.AsyncTask;


/**
 * Action to run in background while a modal progress bar is shown
 */
public class BackgroundAction extends AsyncTask<Void, Void, Void> {
    private ProgressDialog progressBar;
    private Dialog initiator;
    private Runnable runnable;
    private String message;

    /**
     * Creates background action
     * @param initiator     A dialog to be hidden after the progress bar appears
     * @param message       Message shown next to the progress bar
     * @param runnable      The action body
     */
    private BackgroundAction(Dialog initiator, String message, Runnable runnable) {
        progressBar = new ProgressDialog(initiator.getContext());
        this.initiator = initiator;
        this.runnable = runnable;
        this.message = message;
    }
    @Override
    protected void onPreExecute() {
        progressBar.setMessage(message);
        progressBar.setCancelable(false);
        progressBar.show();
        initiator.hide();
    }

    @Override
    protected Void doInBackground(Void... voids) {
        runnable.run();
        return null;
    }

    @Override
    protected void onPostExecute(Void result) {
        if (progressBar.isShowing())
            progressBar.dismiss();
    }

    static void run(Dialog initiator, String message, Runnable runnable) {
        new BackgroundAction(initiator, message, runnable).execute();
    }
}
