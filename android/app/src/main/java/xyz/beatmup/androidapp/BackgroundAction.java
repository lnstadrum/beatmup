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
