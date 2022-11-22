#/bin/sh
# move the msyncs configs, logs etc from .cache to .local
if [ ! -a $HOME/.local/share/system/privileged/msyncd/cache_dir_migrated ]; then
  if [ -d $HOME/.cache/msyncd ]; then
    if [ -d $HOME/.local/share/system/privileged/msyncd/sync ]; then
      # msyncd probably restarted before this oneshot got executed. just move basic known content
      mkdir -p $HOME/.local/share/system/privileged/msyncd/sync/logs
      mv $HOME/.cache/msyncd/sync/*.xml $HOME/.local/share/system/privileged/msyncd/sync
      mv $HOME/.cache/msyncd/sync/logs/*.xml $HOME/.local/share/system/privileged/msyncd/sync/logs/

    else
      mv $HOME/.cache/msyncd $HOME/.local/share/system/privileged
    fi
  fi

  mkdir -p $HOME/.local/share/system/privileged/msyncd
  touch $HOME/.local/share/system/privileged/msyncd/cache_dir_migrated
fi
