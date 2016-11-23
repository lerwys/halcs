package br.lnls.dig.gradle.nativedistribution.model.internal

import org.gradle.api.distribution.internal.DefaultDistribution
import org.gradle.api.file.CopySpec
import org.gradle.api.Task
import org.gradle.nativeplatform.BuildType
import org.gradle.nativeplatform.Flavor
import org.gradle.nativeplatform.NativeBinarySpec
import org.gradle.nativeplatform.NativeExecutableBinarySpec
import org.gradle.nativeplatform.SharedLibraryBinarySpec
import org.gradle.platform.base.Platform

import br.lnls.dig.gradle.nativedistribution.model.DistributionVariant
import br.lnls.dig.gradle.sysfiles.model.SysFilesSet

public class DefaultDistributionVariant extends DefaultDistribution
        implements DistributionVariant {
    BuildType buildType
    Flavor flavor
    Platform platform
    String usage

    Set<NativeExecutableBinarySpec> executables
    Set<SharedLibraryBinarySpec> sharedLibraries
    Set<NativeBinarySpec> binariesWithSysFiles

    DefaultDistributionVariant(String name, CopySpec spec) {
        super(name, spec)

        executables = new LinkedHashSet<>()
        sharedLibraries = new LinkedHashSet<>()
        binariesWithSysFiles = new LinkedHashSet<>()
    }

    void addBinaries(Iterable<NativeBinarySpec> binaries) {
        def compatibleBinaries = binaries.findAll() { binary ->
            binary.buildType == buildType &&
            binary.flavor == flavor &&
            binary.targetPlatform == platform
        }

        compatibleBinaries.each { binary ->
            if (binary instanceof NativeExecutableBinarySpec)
                executables.add((NativeExecutableBinarySpec)binary)
            if (binary instanceof SharedLibraryBinarySpec)
                sharedLibraries.add((SharedLibraryBinarySpec)binary)
            if (hasSysFilesSet(binary))
                binariesWithSysFiles.add(binary)
        }
    }

    boolean hasSysFilesSet(NativeBinarySpec binary) {
        def sysFilesSets = binary.inputs.findAll { it instanceof SysFilesSet }

        return !sysFilesSets.isEmpty()
    }

    @Override
    public Set<Task> getBuildTasks() {
        return executables.collect { it.buildTask }
            + sharedLibraries.collect { it.buildTask }
    }
}
