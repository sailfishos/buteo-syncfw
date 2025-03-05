TEMPLATE = subdirs

SUBDIRS += \
    libbuteosyncfw \
    declarative \
    msyncd \
    oopp-runner \
    unittests \
    doc

declarative.depends = libbuteosyncfw
msyncd.depends = libbuteosyncfw
oopp-runner.depends = libbuteosyncfw
unittests.depends = libbuteosyncfw msyncd
