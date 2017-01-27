package br.lnls.dig.gradle.nativedistribution.tasks.internal

import org.gradle.api.file.FileCollection

import br.lnls.dig.gradle.nativedistribution.tasks.Rpm

class RpmArchiveHeadersAction extends AbstractRpmArchiveAction  {
    FileCollection exportedHeaders

    public RpmArchiveHeadersAction(Rpm rpmTask) {
        super(rpmTask)

        exportedHeaders = rpmTask.exportedHeaders
    }

    @Override
    protected String getPackageName() {
        return projectName + "-devel"
    }

    @Override
    protected void addArchiveFiles() {
        exportedHeaders.each { headerFile -> addHeaderFile(headerFile) }
    }

    private void addHeaderFile(File headerFile) {
        def headerFilePath = "$installationPrefix/include/$headerFile.name"

        rpmBuilder.addFile(headerFilePath, headerFile)
    }
}