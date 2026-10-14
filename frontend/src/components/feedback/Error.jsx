import ErrorIcon from "../../assets/icons/x.svg?react";

const Error = ({ errorMessage }) => {
  return (
    <div className="flex flex-row items-center justify-start w-auto gap-1 py-4 px-3 rounded-[20px] bg-surface/80">
      <ErrorIcon className={"h-7 w-7 text-danger"} />
      <p>{errorMessage}</p>
    </div>
  );
};

export default Error;
