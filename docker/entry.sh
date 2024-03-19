## User install
addgroup --gid ${USER_GID} ${USER_NAME}
useradd ${USER_NAME} -u${USER_UID} -g${USER_GID} -s /bin/bash
su ${USER_NAME}
