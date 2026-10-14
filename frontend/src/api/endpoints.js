const endpoints = {
  auth: {
    login: "/users/login",
    logout: "/users/logout",
    me: "/users/me",
    profile: "/users/me",
    password: "/users/me/password",
  },

  users: {
    list: "/users",
    create: "/users",
    detail: (id) => `/users/${id}`,
  },

  agents: {
    list: "/agents",
    create: "/agents",
    detail: (id) => `/agents/${id}`,
    logs: (id) => `/agents/${id}/logs`,
    alerts: (id) => `/agents/${id}/alerts`,
  },

  logs: {
    list: "/logs",
    detail: (id) => `/logs/${id}`,
  },

  notifications: {
    list: "/notifications",
    create: "/notifications",
    detail: (id) => `/notifications/${id}`,
    archive: (id) => `/notifications/${id}/archive`,
  },
};

export default endpoints;
